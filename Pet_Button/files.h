
const char back_file[] PROGMEM = "back";
const char drink_file[] PROGMEM = "drink";
const char good_job_file[] PROGMEM = "good_job";
const char mousi_file[] PROGMEM = "mousi";
const char panther_file[] PROGMEM = "panther";
const char tail_file[] PROGMEM = "tail";
const char belly_file[] PROGMEM = "belly";
const char ear_file[] PROGMEM = "ear";
const char head_file[] PROGMEM = "head";
const char naoko_file[] PROGMEM = "naoko";
const char paw_file[] PROGMEM = "paw";
const char toy_file[] PROGMEM = "toy";
const char box_file[] PROGMEM = "box";
const char eat_file[] PROGMEM = "eat";
const char hmm_file[] PROGMEM = "hmm";
const char neck_file[] PROGMEM = "neck";
const char pet_file[] PROGMEM = "pet";
const char pets_file[] PROGMEM = "pets";
const char treats_file[] PROGMEM = "treats";
const char carsten_file[] PROGMEM = "carsten";
const char eye_file[] PROGMEM = "eye";
const char later_file[] PROGMEM = "later";
const char no_file[] PROGMEM = "no";
const char play_file[] PROGMEM = "play";
const char walk_file[] PROGMEM = "walk";
const char catnip_file[] PROGMEM = "catnip";
const char feather_file[] PROGMEM = "feather";
const char love_file[] PROGMEM = "love";
const char oops_file[] PROGMEM = "oops";
const char potty_file[] PROGMEM = "potty";
const char water_file[] PROGMEM = "water";
const char cats_file[] PROGMEM = "cats";
const char food_file[] PROGMEM = "food";
const char mad_file[] PROGMEM = "mad";
const char ouch_file[] PROGMEM = "ouch";
const char pedme_file[] PROGMEM = "Pedme";
const char princess_file[] PROGMEM = "princess";
const char yes_file[] PROGMEM = "yes";
const char chase_file[] PROGMEM = "chase";
const char go_file[] PROGMEM = "go";
const char mom_file[] PROGMEM = "mom";
const char outside_file[] PROGMEM = "outside";
const char puzzle_file[] PROGMEM = "puzzle";
const char dad_file[] PROGMEM = "dad";
const char good_file[] PROGMEM = "good";
const char mona_file[] PROGMEM = "mona";
const char padme_file[] PROGMEM = "padme";
const char sleep_file[] PROGMEM = "sleep";
const char what_file[] PROGMEM = "what";
const char want_file[] PROGMEM = "want";
const char where_file[] PROGMEM = "where";
const char like_file[] PROGMEM = "like";
const char snuggles_file[] PROGMEM = "snuggles";

/*
snuggles  mad       hmm
treats    play      walk
food      pets      outside
==============================
what      want      where      
like      panther   pedme
yes       no        later
==============================
naoko     carsten   cats
oops      ouch      water
eat       go        love
*/


const char* files[9*6] PROGMEM = {
  snuggles_file,   // 0
  treats_file,     // 1
  food_file,       // 2
  0,               // 3
  0,               // 4
  0,               // 5
  0,               // 6
  0,               // 7
  0,               // 8
  mad_file,        // 9
  play_file,       // 10
  pets_file,       // 11
  0,               // 12
  0,               // 13
  0,               // 14
  0,               // 15
  0,               // 16
  0,               // 17
  hmm_file,        // 18
  walk_file,       // 19
  outside_file,    // 20
  0,               // 21
  0,               // 22
  0,               // 23
  0,               // 24
  0,               // 25
  0,               // 26
  0,               // 27
  0,               // 28
  0,               // 29
  what_file,       // 30
  like_file,       // 31
  yes_file,        // 32
  0,               // 33
  0,               // 34
  0,               // 35
  0,               // 36
  0,               // 37
  0,               // 38
  want_file,       // 39
  panther_file,    // 40
  no_file,         // 41
  0,               // 42
  0,               // 43
  0,               // 44
  0,               // 45
  0,               // 46
  0,               // 47
  where_file,      // 48
  pedme_file,      // 49
  later_file,       // 50
  0,               // 51
  0,               // 52
  0                // 53
};


/*
const char* files[9*6] PROGMEM = {
  panther_file,    // 0
  go_file,         // 1
  outside_file,    // 2
  0,               // 3
  0,               // 4
  0,               // 5
  0,               // 6
  0,               // 7
  0,               // 8
  princess_file,   // 9
  eat_file,        // 10
  food_file,       // 11
  0,               // 12
  0,               // 13
  0,               // 14
  0,               // 15
  0,               // 16
  0,               // 17
  cats_file,       // 18
  play_file,       // 19
  toy_file,        // 20
  0,               // 21
  0,               // 22
  0,               // 23
  0,               // 24
  0,               // 25
  0,               // 26
  0,               // 27
  0,               // 28
  0,               // 29
  hmm_file,        // 30
  pet_file,        // 31
  walk_file,       // 32
  0,               // 33
  0,               // 34
  0,               // 35
  0,               // 36
  0,               // 37
  0,               // 38
  later_file,      // 39
  love_file,       // 40
  yes_file,        // 41
  0,               // 42
  0,               // 43
  0,               // 44
  0,               // 45
  0,               // 46
  0,               // 47
  oops_file,       // 48
  mad_file,        // 49
  no_file,         // 50
  0,               // 51
  0,               // 52
  0                // 53
};

*/
