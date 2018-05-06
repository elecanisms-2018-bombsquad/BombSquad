/*
 * Codeword module header file for forward declarations
 * Alexander Hoppe, Elecanisms 2018
 */

#ifndef _I2C_H_
#define _I2C_H_

void updateDisplay(void);
void toggleSwitchSetup(void);

const char words[35][6] = {
    "about",
    "every",
    "large",
    "plant",
    "spell",
    "these",
    "where",
    "after",
    "first",
    "learn",
    "point",
    "still",
    "thing",
    "which",
    "again",
    "found",
    "never",
    "right",
    "study",
    "think",
    "world",
    "below",
    "great",
    "other",
    "small",
    "their",
    "three",
    "would",
    "could",
    "house",
    "place",
    "sound",
    "there",
    "water",
    "write"
};

const char letterSets[15][36] = {
    "WRITEGYQWVMRTEPIXZFMIJYJTFPBAMEQPGX",
    "STILLSGCBWTRTVZSJVIDRFPLJOKUSIJLAFZ",
    "EVERYKEXMYZMSCTVPAGNDEFDHERQJHMWNYX",
    "WORLDGWQAPNWEOKMQQWRZYJPDLJBINSICDH",
    "WRITEGWJXDLZPLXIRJTCIFSTFHBIOSQXUJE",
    "FOUNDZIJCEFOGMQYNQBVEUJZNOUWJQGLIDP",
    "COULDDXCUJTOZUPQYMKXVUSLKRQVWMHDVQJ",
    "THESEXTKDLVGHITPSEQIVTMEHDONSVYMEFB",
    "OTHERRIBOCETNRKJBNJUHGEGJVWKEKDVRPM",
    "WORLDTXSURWFWOQYRVRQGNJYBKLQOHXDEIQ",
    "SMALLIVRXSUXDCMRYAQTFPJVPHFLXQLROBH",
    "THESEKPTRLBHKVUNDQGLEJSXAMSVDWAOEFN",
    "WRITEVWIXYHWKXBGRFELYIPUADTRNLVEAQU",
    "GREATLOGMXVYCNRWKHVPEBIZAPWVFWHFSTQ",
    "OTHERFGIODHBTPXSICGELPHADEJIZNAPKIR"
};

#endif
