//
// Created by chris on 1/10/17.
//
[[gnu::always_inline]] [[gnu::const]] constexpr char uncompressOneChar(char val) {
    unsigned uchar = val;
    unsigned step1 = (uchar << 1) & 0xC;
    unsigned step2 = 0xFFFFFFF1 & uchar;
    unsigned step3 = (uchar >> 2) & 0x2;
    unsigned unified = step1 | step2 | step3;
    return unified + 0x24;
}


static char buff[256] = {};
const char* uncompressClassname(const char* __restrict__ s, unsigned len) {
    char* __restrict end = &buff[len];

    for(char*__restrict buffptr = &buff[0]; buffptr < end; ++buffptr, ++s) {
        *buffptr = uncompressOneChar(*s);
    }
    *end = 0;
    return &buff[0];
}