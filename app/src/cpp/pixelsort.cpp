//
// Created by chris on 12/10/16.
//
#include "util/stl.hpp"
#include "glitch-common.h"
#include "pixelsort.h"

#include <algorithm>
#include <jni.h>

#include "CSApplication.h"
using namespace cs;

long long PixelSorter::timeTaken;
unsigned long long PixelSorter::hits_, PixelSorter::misses_;
#include <android/log.h>
extern "C"
void logRecursive(unsigned val) {
    __android_log_print(ANDROID_LOG_VERBOSE, "Glitch Workshop", "Value for recursive call: %d", val);
}


PixelSorter::PixelSorter(int *const colors, int width, int height, unsigned size, sortingMode mode)
        : colors(colors),
          w(width),
          h(height),
          size(size),
          mode(mode),
          buffer(cs::allocate<int>(maxBufferSize)),
          buffer2(cs::allocate<int>(maxBufferSize))

{
}

PixelSorter::~PixelSorter() {
    cs::deallocate(buffer);
}
#define declcase(color, type, var, buffer)      case sortingMode ::color: \
    sort##type##color(this, buffer, var++); \
    break
#if 0
volatile bool waitingForColumns = true;
template<sortingMode mode_>
static void* backgroundTaskColumns(void* ud) {
    waitingForColumns = true;
    int w = ((PixelSorter*)ud)->getW();
    int column = w / 2;
#define this    ((PixelSorter*)ud)
    int* buffer = this->getBuffer2();
    while(column < w - 1) {
        //sortColumn<mode_>(column++);
        switch(mode_) {
            declcase(Black, Column, column, buffer);
            declcase(White, Column, column, buffer);
            declcase(Bright, Column, column, buffer);
        }
    }
    waitingForColumns = false;
    return nullptr;
#undef this
}
#endif

template<sortingMode mode_>
void PixelSorter::sortMain() {
    int column = 0, row = 0;

    /*pthread_t halfthread = 0;
    pthread_create(&halfthread, nullptr, &backgroundTaskColumns<mode_>, this);
    int halfw = w / 2;
    while(column < halfw - 1) {*/
    while(column < w - 1) {
        //sortColumn<mode_>(column++);
        switch(mode_) {
            declcase(Black, Column, column, buffer);
            declcase(White, Column, column, buffer);
            declcase(Bright, Column, column, buffer);
        }
    }
    /*if(waitingForColumns)
        pthread_join(halfthread, nullptr);*/
    while(row < h - 1) {
        switch (mode_) {

            declcase(Black, Row, row, buffer);
            declcase(White, Row, row, buffer);
            declcase(Bright, Row, row, buffer);
        }

    }
}
void PixelSorter::sort() {
    auto timeStart = CSApplication::getMilliseconds();
    switch(mode) {
        case sortingMode::Black:
            sortMain<sortingMode::Black>();
            break;
        case sortingMode ::Bright:
            sortMain<sortingMode::Bright>();
            break;
        case sortingMode ::White:
            sortMain<sortingMode::White>();
            break;
    }
    timeTaken = CSApplication::getMilliseconds() - timeStart;
    hits_ = hits;
    misses_ = misses;
}
