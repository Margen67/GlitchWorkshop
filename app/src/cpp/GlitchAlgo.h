#pragma once

#include <NativelyCachedBmp.h>

enum class Algos : unsigned {
    None = 0,
    ALGO_XOR = 1,
    ALGO_OR = 2,
    ALGO_AND = 3,
    ALGO_ADD = 4,
    ALGO_SUB = 5,
    ALGO_MUL = 6,
    ALGO_DIV = 7,
    ALGO_DIV64 = 8,
    ALGO_REVSUB = 9,
    ALGO_RBIT = 10,
    ALGO_RBYTE = 11,
    ALGO_PIXELSORT = 12
};

enum class SrcChoice : int {
    Random,
    Constant,
    Text,
    Image
};
enum class DataMode : int {
    Pixel,
    Raw,
    BAD = -1
};

enum class RawFormat_t : int {
    none,
    jpg,
    png
};

struct GlitchAlgo {
    CSApplication* app;
    DataMode datamode;
    Algos algo;
    SrcChoice srcChoice;
    unsigned constantVal;
    jstring jstrTextVal;
    cloture::util::string::String_t textVal;
    jobject jresizedDataSrcBmp;
    NativelyCachedBmp* resizedDataSrcBmp;
    volatile pthread_t backgroundThread = 0;
    NativelyCachedBmp* currentUnresizedCopy = nullptr;
    NativelyCachedBmp* volatile backgroundResult = nullptr;
    bool canUseLastResult = false;
    bool preserveTransparency = true;
    FILE* volatile currentRaw = nullptr;
    cloture::util::string::String_t* volatile rawPath;
    sortingMode sortingmode = sortingMode::Black;
    //cloture::util::vector::Vector_t<NativelyCachedBmp*> undoHistory;
    NativelyCachedBmp** undoHistory = nullptr;
    volatile unsigned undoIndex = 0;
    size_t undoMax = 14;

    NativelyCachedBmp** redoHistory = nullptr;
    volatile unsigned redoIndex = 0;
    size_t redoMax = 14;

    GlitchAlgo();
    unsigned getRandomVal();
    void processFrameData(cloture::util::common::uint8* buff, int w, int h, size_t buffsize);
    void bend(int* data, int w, int h, size_t sz);
    void setTextVal(JNIEnv* env, jstring str);
    void setResizedBmp(JNIEnv* env, jobject bmp, jlong cachePtr);
    void setDataSrc(JNIEnv* env, SrcChoice choice);
    bool getPreserveTransparency();
    void setPreserveTransparency(bool b);
    void backgroundPreprocessAndCache();
    void notifyBitmapChange(NativelyCachedBmp*);
    void openRaw(cloture::util::string::String_t filepath);
    /*
     * creates a duplicate of newUndo and adds it to the history
     *
     */
    void addToUndo(NativelyCachedBmp* newUndo, bool clear_redo = true);
    /*
     * frees all of the bitmaps in the history and zeroes the array
     */
    void clearUndoHistory();

    NativelyCachedBmp* popUndo();

    void addToRedo(NativelyCachedBmp* newRedo);
    void clearRedoHistory();
    NativelyCachedBmp* popRedo();
};

extern GlitchAlgo glitchAlgo;