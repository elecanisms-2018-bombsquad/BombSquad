
void disp_mhz(uint16_t reading);

void dispSeconds(uint16_t seconds);
void doMorse(char* morse_str);

void dispNumber(uint16_t number);

const char morse_table[15][32] = {
    "... .... . .-.. .-..", //shell
    ".... .- .-.. .-.. ...", //halls
    "... .-.. .. -.-. -.-", //slick
    "- .-. .. -.-. -.-", //trick
    "-... --- -..- . ...", //boxes
    ".-.. . .- -.- ...", //leaks
    "... - .-. --- -... .", //strobe
    "-... .. ... - .-. ---", //bistro
    "..-. .-.. .. -.-. -.-", //flick
    "-... --- -- -... ...", //bombs
    "-... .-. . .- -.-", //break
    "-... .-. .. -.-. -.-", //brick
    "... - . .- -.-", //steak
    "... - .. -. --.", //sting
    "...- . -.-. - --- .-."  //vector
};

const uint16_t freq_table[15] = {
    3505, //shell
    3515, //halls
    3522, //slick
    3532, //trick
    3535, //boxes
    3542, //leaks
    3545, //strobe
    3552, //bistro
    3555, //flick
    3565, //bombs
    3572, //break
    3575, //brick
    3582, //steak
    3592, //sting
    3595  //vector
};
