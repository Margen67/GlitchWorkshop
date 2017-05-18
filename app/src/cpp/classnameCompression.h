//
// Created by chris on 1/10/17.
//
#pragma once
constexpr char compressOneChar(char val) {
    unsigned uchar = val;
    uchar -= 0x24;
    unsigned step1 = (0xC & uchar) >> 1;
    unsigned step2 = 0xFFFFFFF1 & uchar;
    unsigned step3 = (0x2 & uchar) << 2;

    return static_cast<char>( step1 | step2 | step3);
}

constexpr char uncompressOneChar(char val) {
    unsigned uchar = val;
    unsigned step1 = (uchar << 1) & 0xC;
    unsigned step2 = 0xFFFFFFF1 & uchar;
    unsigned step3 = (uchar >> 2) & 0x2;
    unsigned unified = step1 | step2 | step3;
    return unified + 0x24;
}
constexpr double compressionRatio = 6.0 / 8.0;
template<typename T>
constexpr size_t calculateCompressedSize(T x) {
    unsigned len = x.length();
    return static_cast<size_t>(len* compressionRatio) + 1;
}

constexpr cloture::util::common::uint8 rightRotate(cloture::util::common::uint8 n, unsigned int d)
{
    return (n >> d)|(n << (8 - d));
}

constexpr unsigned char masks[] = {
        0b00111111, //first 6 bits for first char
        0b00110000, //mask top 2 of second char to merge into previous,
        0b00001111, //mask bottom 4 of second char
        0b00111100, //mask top 4 of third char to merge into previous,
        0b00000011, //mask bottom 2 of third char
        0b00111111, //mask 6 bits to merge into previous

        //then repeat
};



constexpr unsigned char getMaskForCurrentChar(unsigned index) {
    return masks[index % 6];
}

constexpr unsigned getPackedIndex(unsigned index) {
    return (unsigned)(index * compressionRatio);
}
static_assert(cloture::util::common::findBitSet(masks[0]) == 5);

#if 0
constexpr void compressme(const char* s, char * other, size_t size) {
    using namespace cloture::util::common;
    uint8 mask = 0b00111111;
    uint8 shift = 0;
    bool first = true;
    uint8 lastUnusedBits = 0;
    for(unsigned i = 0; i < size; ++i) {
        if(first) {
            first = false;
            lastUnusedBits = 2;
        }
        else {
            uint8 lastMask = makeMask<uint8>(lastUnusedBits);
            uint8 rotation = 6 - lastUnusedBits;
            uint8 rotatedVal = rightRotate(s[i], rotation);
            other[i - 1] |= (rotatedVal & lastMask);
            lastUnusedBits = 6 - lastUnusedBits;
        }
        shift = lastUnusedBits;
        uint8 x = s[i] & mask;
        other[i] = x << shift;
    }
}
#else

constexpr void compressme(const char* s, char * other, size_t size) {
    using namespace cloture::util::common;
    unsigned previousValue = 0;
    for(unsigned i = 0, mask = getMaskForCurrentChar(i); i < size; ++i, mask = getMaskForCurrentChar(i)) {
        unsigned currentUnpackedChar = s[i];
        unsigned maskedValue = currentUnpackedChar & mask;

        unsigned shiftFactor = 7 - findBitSet(mask);
        unsigned compressedIndex = getPackedIndex(i);
        unsigned shiftedValue = maskedValue << shiftFactor;
        other[compressedIndex] |= shiftedValue;
    }
}


#endif

#define COMPRESS_CLASSNAME(classname)       mMetaStringOp(compressOneChar, classname, 256)::str

constexpr char* validClassnameASCII = "$1234567890./abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr auto compressedValidAscii = mMetaStringOp(compressOneChar, "$1234567890./abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 128)::str;


constexpr bool assertCompressionWorks() {
    for(unsigned i = 0; i < compressedValidAscii.length(); ++i) {
        if(validClassnameASCII[i] != uncompressOneChar(compressedValidAscii[i]))
            return false;
    }
    return true;
}

static_assert(assertCompressionWorks());


const char* uncompressClassname(const char* s, unsigned len);
