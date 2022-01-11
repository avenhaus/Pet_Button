//**********************************************************************************************************
//*   Pet Word Player                                                             *
//**********************************************************************************************************
//
// Arduino bootloader is slow to wake up and can not be customized. Flash PlatformIO bootloader:
//  pio run -t menuconfig
// The menuconfig target is only available for ESP-IDF framework and Arduino uses prebuilt ESP-IDF so it's not configurable.
// pio run -v -t upload
// C:\Users\caavenha\.platformio\packages\tool-esptoolpy\esptool.py --chip esp32 --port "COM5" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 C:\Repos\ESP32\Pet-Button\.pio\build\esp32dev\bootloader.bin

#include "Arduino.h"
#include "WiFiMulti.h"
#include "driver/adc.h"
#include <esp_bt.h>
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "SPIFFS.h"
#include "driver/rtc_io.h"


// Idle Power: 
// 85uA with ULN2003 and MAX98357 disabled
// 350uA with 100K output pullups to  +5V - ESP32 clamping to 3.3V
// 400uA with MAX98357 enabled (1M pullup not removed) and no pullups
// 675uA with 100K pullups and MAX98357 enabled



/********************************************\
|*  Pin Definitions
\********************************************/

// Buttons are arranged on 3 x 3 panels = 9 buttons, requiring 6 wires
// Button matrix is 9 inputs by 6 outputs for a support of 54 Buttons

// Button Inputs - 100K pulldown to allow deep sleep
#define BIN_0   2
#define BIN_1   4
#define BIN_2   12
#define BIN_3   32
#define BIN_4   33
#define BIN_5   34
#define BIN_6   35
#define BIN_7   36
#define BIN_8   39

const uint64_t WAKE_MASK = (1LL<<BIN_0) | (1LL<<BIN_1) | (1LL<<BIN_2) |
                           (1LL<<BIN_3) | (1LL<<BIN_4) | (1LL<<BIN_5) |
                           (1LL<<BIN_6) | (1LL<<BIN_7) | (1LL<<BIN_8);
                           
// Button Open Drain Outputs - 220 pullup to 5V to allow wakeup on any button
#define BOUT_0   13
#define BOUT_1   14
#define BOUT_2   15
#define BOUT_3   16
#define BOUT_4   17
#define BOUT_5   18

#define USE_ULN2003   1


// I2S Audio Amplifier
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

// Amplifier Power Enable - Also Boot button, so use open drain...
#define POWER_EN 19

// SD Card (optional)
#define SD_CS          5
#define SPI_MOSI      21
#define SPI_MISO      22
#define SPI_SCK       23


/* ============================================== *\
 * Debug
\* ============================================== */

#define SERIAL_DEBUG 1
#define SERIAL_SPEED 115200

#if SERIAL_DEBUG < 1
#define DEBUG_println(...) 
#define DEBUG_print(...) 
#define DEBUG_printf(...) 
#else
#define DEBUG_println(...) if (debugStream) {debugStream->println(__VA_ARGS__);}
#define DEBUG_print(...) if (debugStream) {debugStream->print(__VA_ARGS__);}
#define DEBUG_printf(...) if (debugStream) {debugStream->printf(__VA_ARGS__);}
#endif

#define FST (const char *)F
#define PM (const __FlashStringHelper *)

/* ============================================== *\
 * Data
\* ============================================== */

const char version_number[] PROGMEM = "0.2";
const char version_date[] PROGMEM = __DATE__;
const char version_time[] PROGMEM = __TIME__;

const uint8_t KEY_IN[9] = { BIN_0, BIN_1, BIN_2, BIN_3, BIN_4, BIN_5, BIN_6, BIN_7, BIN_8 };
const uint8_t KEY_OUT[7] = { BOUT_0, BOUT_1, BOUT_2, BOUT_3, BOUT_4, BOUT_5 };

static uint64_t old_keys = 0;
static uint64_t keys = 0;

static uint8_t queue[32];
static uint8_t q_size = 0;
static uint8_t q_write = 0;
static uint8_t q_read = 0;

static bool is_playing = false;

static uint32_t time_ms = 0;

Stream* debugStream = &Serial;
Audio audio;

WiFiMulti wifiMulti;
const char ssid[] PROGMEM =     "My-Wifi-Network";
const char password[] PROGMEM = "Wifi-Password";

#include "files.h"

void setup() {
  Serial.begin(SERIAL_SPEED);
  DEBUG_print(F("Cat Button\n"));
  //DEBUG_printf(FST("\n\nPet Buttons Version %s | %s | %s\n\n"), version_number, version_date, version_time);

  for (int i=0; i<9; i++) { pinMode(KEY_IN[i], INPUT); }
#if USE_ULN2003
  for (int i=0; i<6; i++) { pinMode(KEY_OUT[i], OUTPUT); digitalWrite(KEY_OUT[i], LOW); }
#else
  for (int i=0; i<6; i++) { pinMode(KEY_OUT[i], OUTPUT_OPEN_DRAIN); digitalWrite(KEY_OUT[i], HIGH); }
#endif
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[4], PIN_FUNC_GPIO);
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[12], PIN_FUNC_GPIO);

  
  pinMode(POWER_EN, OUTPUT); digitalWrite(POWER_EN, HIGH);

  #if USE_SD
    pinMode(SD_CS, OUTPUT); digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    SD.begin(SD_CS);
  #endif


  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    print_wakeup_reason();
  
    int GPIO_reason = esp_sleep_get_ext1_wakeup_status();
    DEBUG_print(F("GPIO that triggered the wake up: GPIO "));
    DEBUG_println((log(GPIO_reason))/log(2), 0);
  
    memset(queue, 0, sizeof(queue));
    keys = get_keys();
    // DEBUG_printf(FST("Keys: %llX\n"), keys);
  

    if(!SPIFFS.begin(true)){
      DEBUG_print(F("SPIFFS Mount Failed"));
    }
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21
    if (q_size) { play_next(); }
  }
  else {
    if(!SPIFFS.begin(true)){
      DEBUG_print(F("SPIFFS Mount Failed"));
    }
    list_dir(SPIFFS, "/", 0);
    go_sleep();
  }
  time_ms = millis();
}

void loop()
{
  audio.loop();
  uint32_t ms = millis();
  if ( ms - time_ms >= 10 || ms < time_ms ) {
    keys = get_keys();
    time_ms = ms;
  }
  if (!is_playing && !keys) {
    go_sleep();
  }
}


inline void set_out(uint8_t n) {
  // DEBUG_printf(FST("Out: %X\n"), n);
  for (uint8_t i=0; i<6; i++) {
#if USE_ULN2003
    digitalWrite(KEY_OUT[i], !(n & 1));
#else
    digitalWrite(KEY_OUT[i], n & 1);
#endif
    n = n >> 1;
  }
}

inline uint16_t get_in() {
  uint16_t n = 0;
  for (int i=8; i>=0; i--) {
    n = n << 1;
    n |= digitalRead(KEY_IN[i]);
    // DEBUG_printf(FST("(%d %d 0x%X) "), i, KEY_IN[i], n);
  }
  // DEBUG_printf(FST("In: %X\n"), n);
  return n;
}

uint64_t get_keys() {
  for (int i=0; i<9; i++) { pinMode(KEY_IN[i], INPUT); }
  uint64_t keys = 0;
  uint8_t out = 1;
  for (int i=0; i<6; i++) {
    set_out(out);
    out = out << 1;
    keys |= ((uint64_t) get_in()) << (i*9);
  }
  set_out(0xFF); // Go high to not waste power on the pullups
  uint64_t n = (old_keys ^ keys) & keys;
  for (int i=0; i<64; i++) {
    if (n & 1) {
      push(i);    
    }
    n = n >> 1;
  }
  old_keys = keys;
  for (int i=0; i<9; i++) { pinMode(KEY_IN[i], OUTPUT); }
  return keys;
}


void list_dir(fs::FS &fs, const char * dirname, uint8_t levels){
   DEBUG_printf(FST("Listing directory: %s\r\n"), dirname);

   File root = fs.open(dirname);
   if(!root){
      DEBUG_println(F("− failed to open directory"));
      return;
   }
   if(!root.isDirectory()){
      DEBUG_println(F(" − not a directory"));
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         DEBUG_print(F("  DIR : "));
         DEBUG_println(file.name());
         if(levels){
            list_dir(fs, file.name(), levels -1);
         }
      } else {
         DEBUG_print(F("  FILE: "));
         DEBUG_print(file.name());
         DEBUG_print(F("\tSIZE: "));
         DEBUG_println(file.size());
      }
      file = root.openNextFile();
   }
}


void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : DEBUG_print(F("Wakeup caused by external signal using RTC_IO")); break;
    case ESP_SLEEP_WAKEUP_EXT1 : DEBUG_print(F("Wakeup caused by external signal using RTC_CNTL")); break;
    case ESP_SLEEP_WAKEUP_TIMER : DEBUG_print(F("Wakeup caused by timer")); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : DEBUG_print(F("Wakeup caused by touchpad")); break;
    case ESP_SLEEP_WAKEUP_ULP : DEBUG_print(F("Wakeup caused by ULP program")); break;
    default : DEBUG_printf(FST("Wakeup was not caused by deep sleep: %d\n"), wakeup_reason); break;
  }
}

void go_sleep() {
    DEBUG_println(F("Sleep now"));

    digitalWrite(POWER_EN, LOW);
    
    // RTC IO pins: 0,2,4,12-15,25-27,32-39.
    esp_sleep_enable_ext1_wakeup(WAKE_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    
    adc_power_off();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
    btStop();
    

    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    // rtc_gpio_isolate(GPIO_NUM_4);
    // rtc_gpio_isolate(GPIO_NUM_12);
    // delay(100);


    esp_deep_sleep_start(); 
}


void play_next () {
  is_playing = false;
  if (q_size) {
    int8_t b = pop();
    const char* f = files[b];
    if (f) {
      char buffer[64];
      sprintf(buffer, FST("/%s.m4a"), f);   
      DEBUG_printf(FST("Play file: '%s'\n"), buffer);
      audio.connecttoFS(SPIFFS, buffer);
      is_playing = true;
    } else {
      //play_unknown_button(b);
    }
  }
}

void play_unknown_button(uint8_t n) {
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid, password);
    wifiMulti.run();
    if(WiFi.status() != WL_CONNECTED){
      DEBUG_printf(FST("Could not connect to WIFI. Error: %d\n"), WiFi.status());
      WiFi.disconnect(true);
      wifiMulti.run();
      play_next();
      return;
    }
    char buffer[32];
    sprintf(buffer, FST("Button %d"), n);    
    DEBUG_printf(FST("Play TTS: '%s'\n"), buffer);
    audio.connecttospeech(buffer, FST("us"));
    is_playing = true;
}
    

/********************************************\
|* Queue
\********************************************/

inline void push(uint8_t i) {
  if (q_size >= sizeof(queue)) { return; }
  DEBUG_printf(FST("Push: %u\n"), i);
  queue[q_write++] = i;
  if (q_write >= sizeof(queue)) { q_write = 0; }
  q_size++;
}

inline int8_t pop(void) {
  int8_t i = -1;
  if (q_size  == 0) { return -1; }
  i = queue[q_read++];
  if (q_read >= sizeof(queue)) { q_read = 0; }
  q_size--;
  DEBUG_printf(FST("Pop: %u\n"), i);
  return i;
}

/********************************************\
|*  Event Handlers
\********************************************/

void audio_info(const char *info){
    DEBUG_print(F("info        ")); DEBUG_println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    DEBUG_print(F("id3data     ")); DEBUG_println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    DEBUG_print(F("eof_mp3     ")); DEBUG_println(info);
    play_next();
}
void audio_showstation(const char *info){
    DEBUG_print(F("station     ")); DEBUG_println(info);
}
void audio_showstreamtitle(const char *info){
    DEBUG_print(F("streamtitle ")); DEBUG_println(info);
}
void audio_bitrate(const char *info){
    DEBUG_print(F("bitrate     ")); DEBUG_println(info);
}
void audio_commercial(const char *info){  //duration in sec
    DEBUG_print(F("commercial  ")); DEBUG_println(info);
}
void audio_icyurl(const char *info){  //homepage
    DEBUG_print(F("icyurl      ")); DEBUG_println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    DEBUG_print(F("lasthost    ")); DEBUG_println(info);
}
void audio_eof_speech(const char *info){
    DEBUG_print(F("eof_speech  ")); DEBUG_println(info);
    play_next();
}
