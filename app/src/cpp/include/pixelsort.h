//
// Created by chris on 12/10/16.
//

#ifndef IMAGEBENDER_PIXELSORT_H_H
#define IMAGEBENDER_PIXELSORT_H_H
//preoptimization - 1.9mb image took 8158 milliseconds
enum class sortingMode : unsigned {
    Black,
    Bright,
    White
};
#define PIXELSORT_INLINE    __forceinline
//fixedpoint seems to actually hamper performance
//nevermind, when its forced to be inline it improes performance greatly
#define FIXEDPOINT_BRIGHTNESS   1
#define	declsortmode(color)	\
extern "C" void sortRow##color (PixelSorter* thiz, int* buff, int row)

class PixelSorter {
    static constexpr size_t maxBufferSize = 32767;
    const int w, h;
    int* const colors;
    unsigned size;

    sortingMode mode;
    int* buffer;
    int* buffer2;

    //void sortColumn(int column);
    //void sortRow(int row);
    template<sortingMode mode_> void sortMain();
    /*
    template<sortingMode mode_> void sortColumn(int column) __restrict;
    template<sortingMode mode_> void sortRow(int row);*/
    PIXELSORT_INLINE int getFirstNotBlackX(int _x, int _y) const __restrict;
    PIXELSORT_INLINE int getNextBlackX(int _x, int _y) const __restrict;

    PIXELSORT_INLINE int getFirstNotBlackY(int _x, int _y) const __restrict;
    PIXELSORT_INLINE int getNextBlackY(int _x, int _y) const __restrict;


public:
    PixelSorter(int* const colors, int width, int height, unsigned size, sortingMode mode);
    ~PixelSorter();
    int getW(){return w;}
    int* getBuffer2() {return buffer2;}
    void sort();
    static long long timeTaken;
    static unsigned long long hits_;
    static unsigned long long misses_;
};
declsortmode(Black);
declsortmode(White);
declsortmode(Bright);
#define	declsortmode(color)	\
extern "C" void sortColumn##color (PixelSorter* thiz, int* buff, int row)
declsortmode(Black);
declsortmode(White);
declsortmode(Bright);
#endif //IMAGEBENDER_PIXELSORT_H_H

extern "C" unsigned long long hits;
extern "C" unsigned long long misses;