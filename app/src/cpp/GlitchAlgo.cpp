//
// Created by chris on 11/23/16.
//
#include <util/stl.hpp>
#include <jni.h>
#include "glitch-common.h"
#include <NativelyCachedBmp.h>
#include "pixelsort.h"
#include "jniHelpers.h"

#if defined(USE_ARM_EXTENSIONS)
#include <arm_neon.h>
#endif
#include "CSApplication.h"
#include "NativelyCachedBmp.h"
#include <cstdio>
#include "GlitchAlgo.h"
#include "CSPrefs.h"
#include "BitmapUtils.h"

#include "ArmEmitter.h"
#define rawModeDisabled     true
using namespace cloture::util;
static bool waitForCompletion(pthread_t t, void* f) {
    //while(glitchAlgo.backgroundThread)
    //    ;
    pthread_join(t, nullptr);
    return false;
}

template<int (*func)(int, int*)>
static void processValueArray(jint* RESTRICT array aligned_ptr(16), jsize arraySize, int extraVal) {
        for (unsigned i = 0; i < arraySize; ++i) {
            //prefetch(&array[i+8]);
            func(extraVal, &array[i]);
        }
}

template<int (*func)(int, int*)>
static void processValueArray(jint* RESTRICT array aligned_ptr(16), jsize arraySize, const char* extraVal) {
    size_t slen = strlen(extraVal);
    if(unlikely(!slen))
        return;

    unsigned slen_u = slen / sizeof(int);
    if(unlikely(slen_u == 0)) {
        int val = 0;
        assume(slen < 5 && slen >= 1);
        for(unsigned i = 0; i < slen; ++i) {
            val |= (extraVal[i] << (8*i));
        }
        processValueArray<func>(array, arraySize, val);
        return;
    }
    for(unsigned i = 0; i < arraySize; ++i) {
        //modulo is horrendously slow on arm but im a bit pressed for time here
        func(((const int*)extraVal)[i % slen], &array[i]);
    }
}

template<int (*func)(int, int*)>
static void processValueArray(jint* RESTRICT array aligned_ptr(16), jsize arraySize, NativelyCachedBmp* extraVal) {

    jint* otherarray = extraVal->getPixels();
    for(unsigned i = 0; i < arraySize; ++i) {
        func(otherarray[i], &array[i]);
        //prefetch(&otherarray[i+4]);
        //prefetch(&array[i+4]);
    }

}

static void reverseSubImage(jint* RESTRICT array aligned_ptr(16), jsize arraySize) {
    for(unsigned i = 0, j = arraySize - 1; i < arraySize; ++i, --j)
        array[i] -= array[j];
}

#if defined(USE_ARM_EXTENSIONS)
 void fastXor(jint* RESTRICT array aligned_ptr(16), jsize arraySize, int val) {

    cs::u32 scratch;
    cs::u32 counter = 0;
    int32x4_t v1, v2;
    arraySize *= 4;
    asm volatile (

    "vdup.i32 %[xormask], %[val];"
    "l1:;"
    "vld1.i32 %q[vec2], [%[arr]];"
    "veor.i32 %q[vec2], %q[xormask];"
    "vst1.i32 %q[vec2], [%[arr]];"
    "add %[counter], %[counter], #16;"
    "add %[arr], %[arr], #16;"
    "cmp %[counter], %[sz];"
    "blo l1;"
    : [scratch] "=r" (scratch), [counter] "+r" (counter), [xormask] "=w" (v1),
			[vec2] "=w" (v2)
    : [arr] "r" (array), [sz] "r" (arraySize), [val] "r" (val)
    : "cc"
    );
}
#endif


#define QuickOp(opname, op)         static __forceinline int opname (int val, int* RESTRICT result aligned_ptr(16)) { return op; }

QuickOp(doXor, (*result ^= val));
QuickOp(doOr, (*result |= val));
QuickOp(doAnd, (*result &= val));
QuickOp(doAdd, (*result += val));
QuickOp(doSub, (*result -= val));
QuickOp(doMul, (*result *= val));
QuickOp(doDiv, (*result /= val));
QuickOp(doDiv64, (*result = unsigned((((unsigned long long)(*result)) << 32ULL) / ((unsigned long long)val ))) );
QuickOp(doBitRev, (*result = rbit(*result)));
QuickOp(doByteRev, (*result = rbyte(*result)));

GlitchAlgo glitchAlgo;

#define glitchalgo_setter(name, field)       extern "C" \
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_set##name)(JNIEnv* env, jclass thiz, decltype(glitchAlgo.field) val) { \
    glitchAlgo.field = val; \
}
#define glitchalgo_getter(name, field)       extern "C" \
JNIEXPORT decltype(glitchAlgo.field) JNICALL makeJniName(GlitchAlgo_get##name)(JNIEnv* env, jclass thiz) { \
    return glitchAlgo.field; \
}

#define glitchalgo_callsetter(name, field)       extern "C" \
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_set##name)(JNIEnv* env, jclass thiz, decltype(glitchAlgo.field) val) { \
    glitchAlgo.set##name (val); \
}

#define glitchalgo_callgetter(name, field)       extern "C" \
JNIEXPORT decltype(glitchAlgo.field) JNICALL makeJniName(GlitchAlgo_get##name)(JNIEnv* env, jclass thiz) { \
    return glitchAlgo.get##name (); \
}

#define glitchalgo_callsettertype(name, field, t)       extern "C" \
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_set##name)(JNIEnv* env, jclass thiz, t val) { \
    glitchAlgo.set##name (val); \
}

#define glitchalgo_callgettertype(name, field, t)       extern "C" \
JNIEXPORT t JNICALL makeJniName(GlitchAlgo_get##name)(JNIEnv* env, jclass thiz) { \
    return glitchAlgo.get##name (); \
}

//glitchalgo_setter(Algo, algo)
extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_setAlgo)(JNIEnv* env, jclass thiz, decltype(glitchAlgo.algo) val) {
    if(val == glitchAlgo.algo)
        return;

    glitchAlgo.algo = val;
    getCSAPP()->prefsPutInt("last_algo_selection", generic::enumValue(val));
    glitchAlgo.canUseLastResult = false;
}
glitchalgo_getter(Algo, algo)
glitchalgo_getter(DataSrc, srcChoice)
//glitchalgo_setter(DataSrc, srcChoice)
glitchalgo_getter(Const, constantVal)
//glitchalgo_setter(Const, constantVal)

extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_setConst)(JNIEnv* env, jclass thiz, decltype(glitchAlgo.constantVal) val) {
    if(val == glitchAlgo.constantVal)
        return;
    glitchAlgo.constantVal = val;
    glitchAlgo.canUseLastResult = false;
}

void GlitchAlgo::setDataSrc(JNIEnv* env, SrcChoice choice) {
    if(choice == srcChoice)
        return;
    switch(srcChoice) {
        case SrcChoice::Random ... SrcChoice::Constant:
            break;
        case SrcChoice::Image:
            setResizedBmp(env, nullptr, 0L);
            break;
        case SrcChoice::Text:
            setTextVal(env, nullptr);
            break;
    }
    srcChoice = choice;
    canUseLastResult = false;
}
glitchalgo_getter(Datamode, datamode)
//glitchalgo_setter(Datamode, datamode)
extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_setDatamode)(JNIEnv* env, jclass thiz, DataMode val) {
    if(val == glitchAlgo.datamode)
        return;
    if(glitchAlgo.backgroundThread)
        waitForCompletion(glitchAlgo.backgroundThread, nullptr);
    if(rawModeDisabled)
    {
        glitchAlgo.datamode = DataMode ::Pixel;
        return;
    }
    glitchAlgo.datamode = val;
    getCSAPP()->prefsPutInt("datamode", generic::enumValue(val));
    if(val == DataMode::Pixel && glitchAlgo.currentRaw) {
        fclose(glitchAlgo.currentRaw);
        glitchAlgo.currentRaw = nullptr;
        if(glitchAlgo.rawPath)
        {
            delete glitchAlgo.rawPath;
            glitchAlgo.rawPath = nullptr;
        }
    }
    glitchAlgo.canUseLastResult = false;

}
extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_setDataSrc)(JNIEnv* env, jclass thiz, SrcChoice choice) {
    glitchAlgo.setDataSrc(env, choice);
}

GlitchAlgo::GlitchAlgo()
        :   app(nullptr),
            algo(Algos::ALGO_XOR),
            srcChoice(SrcChoice::Random),
            constantVal(0),
            jstrTextVal(nullptr),
            textVal(),
            jresizedDataSrcBmp(nullptr),
            resizedDataSrcBmp(nullptr),
            datamode(DataMode::BAD)
{

}
/*static unsigned insns_[] = {0x0000A0E3,
                            0x1EFF2FE1};*/

//constexpr const char* insnTest = "mov r0, #0; bx lr;";
//dbgFunctionMacro(mMetaString("mov r0, #0; bx lr;", 16));

/*__unused
static constexpr auto insnTest = mMetaString("mov r0, #0; bx lr;", 64)::str;*/

//unsigned testlen = cs::lengthOfString(insnTest);


extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_initNative)(JNIEnv* env, jclass thiz) {
    {
        /*
        for(int i = 0; i < (sizeof(insns_) / sizeof(insns_[0])); ++i)
            insns_[i] = rbyte(insns_[i]);

        cs::ArmEmitter *emitter = new cs::ArmEmitter(insns_, 2);

        int result = (*emitter)();
        delete emitter;*/
        //void* func = emitter->getFunc();
        /*asm volatile("blx %[func]"
        :
        : [func] "r" (func)
        );*/
        //insns_[0] = result;
        //using loltype = mMetaString("mov r0, #0; bx lr;", 64);
        //cs::ArmAssembler<loltype> test;
        //static constexpr cs::ArmAssembler<mMetaString("mov r0, #0; bx lr;", 64)> test;
        //cs::ArmEmitter *emitter = new cs::ArmEmitter(test);
        //int result = (*emitter)();
        //delete emitter;
    }

    glitchAlgo.app = getCSAPP();
    CSApplication& app = *glitchAlgo.app;
    glitchAlgo.undoHistory = cs::allocate<NativelyCachedBmp*>(1024);
    cs::zeroMem(glitchAlgo.undoHistory, 1024);
    glitchAlgo.redoHistory = cs::allocate<NativelyCachedBmp*>(1024);
    cs::zeroMem(glitchAlgo.redoHistory, 1024);
    int dmode = app.prefsGetInt("datamode");
    if(dmode == -1) //first time running, create the key
    {
        app.prefsPutInt("datamode", generic::enumValue(DataMode::Pixel));
        glitchAlgo.datamode = DataMode::Pixel;
    }
    else {
        if(rawModeDisabled)
            glitchAlgo.datamode = DataMode ::Pixel;
        else
            glitchAlgo.datamode = *reinterpret_cast<DataMode *>(&dmode);
    }
    int preserve_trans = app.prefsGetInt("preserve_transparency");
    if(preserve_trans == -1) {
        glitchAlgo.setPreserveTransparency(true);
    }
    else
        glitchAlgo.setPreserveTransparency((bool)preserve_trans);

    int algo = app.prefsGetInt("last_algo_selection");
    if(algo == -1)
        glitchAlgo.algo = Algos::None;
    else
        glitchAlgo.algo = *reinterpret_cast<Algos *>(&algo);

}

void GlitchAlgo::setTextVal(JNIEnv* env, jstring str) {
    if(str == jstrTextVal)
        return;

    if(str) {
        if(jstrTextVal) {
            env->DeleteGlobalRef(jstrTextVal);
            textVal.~String_t();
        }

        jstrTextVal = (jstring)env->NewGlobalRef(str);
        textVal = app->jstringToUTF(str);

    } else {
        env->DeleteGlobalRef(jstrTextVal);
        jstrTextVal = nullptr;
        textVal.~String_t();
    }
    canUseLastResult = false;
}

extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_setTextVal)(JNIEnv* env, jclass thiz, jstring str) {
    glitchAlgo.setTextVal(env, str);
}

glitchalgo_getter(TextVal, jstrTextVal)

void GlitchAlgo::setResizedBmp(JNIEnv* env, jobject bmp, jlong cachePtr) {
    if(bmp == jresizedDataSrcBmp)
        return;

    if(bmp) {
        if(jresizedDataSrcBmp) {
            env->DeleteGlobalRef(jresizedDataSrcBmp);
            //delete resizedDataSrcBmp;
        }

        jresizedDataSrcBmp = env->NewGlobalRef(bmp);
        resizedDataSrcBmp = NativelyCachedBmp::decodeLongPtr(cachePtr);

    }else {
        env->DeleteGlobalRef(jresizedDataSrcBmp);
        //delete resizedDataSrcBmp;
        jresizedDataSrcBmp = nullptr;
        resizedDataSrcBmp = nullptr;
    }
    canUseLastResult = false;
}

extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_setResizedSrcBmp)(JNIEnv* env, jclass thiz, jobject bmp, jlong cachePtr) {
    glitchAlgo.setResizedBmp(env, bmp, cachePtr);
}

glitchalgo_getter(ResizedSrcBmp, jresizedDataSrcBmp)

glitchalgo_getter(SortingMode, sortingmode)
glitchalgo_setter(SortingMode, sortingmode)

template<typename T>
void processArray(T extraVal, const Algos &algo, int w, int h, jsize asz, jint *arr);

template<typename T>
void processArray(T extraVal, const Algos &algo, int w, int h, jsize asz, jint *arr) {
    switch(algo) {
        case Algos::ALGO_XOR:
            processValueArray<doXor>(arr, asz, extraVal);
            break;
        case Algos::ALGO_OR:
            processValueArray<doOr>(arr, asz, extraVal);
            break;
        case Algos::ALGO_AND:
            processValueArray<doAnd>(arr, asz, extraVal);
            break;
        case Algos::ALGO_ADD:
            processValueArray<doAdd>(arr, asz, extraVal);
            break;
        case Algos::ALGO_SUB:
            processValueArray<doSub>(arr, asz, extraVal);
            break;
        case Algos::ALGO_MUL:
            processValueArray<doMul>(arr, asz, extraVal);
            break;
        case Algos::ALGO_DIV:
            processValueArray<doDiv>(arr, asz, extraVal);
            break;
        case Algos::ALGO_DIV64:
            processValueArray<doDiv64>(arr, asz, extraVal);
            break;
        case Algos::ALGO_REVSUB:
            reverseSubImage(arr, asz);
            break;
        case Algos::ALGO_RBIT:
            processValueArray<doBitRev>(arr, asz, extraVal);
            break;
        case Algos::ALGO_RBYTE:
            processValueArray<doByteRev>(arr, asz, extraVal);
            break;
        case Algos::ALGO_PIXELSORT: {
            PixelSorter psort(arr, w, h, asz, glitchAlgo.sortingmode);
            psort.sort();
            break;
        }

    }
}

QuickOp(MakeOpaque, (reinterpret_cast<Color_u*>(result)->col.a = 0xFF))

extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_maxoutAlphaNative)(JNIEnv* env, jobject thiz, jintArray java_array){
    jsize asz;
    jint* arr = acquireIntArray(env, java_array, asz);
    processValueArray<MakeOpaque>(arr, asz, 0);
    env->ReleaseIntArrayElements(java_array, arr, 0);
}

void GlitchAlgo::bend(int* data, int w, int h, size_t sz) {
    switch(srcChoice) {
        case SrcChoice::Random:
            processArray<int>((int)getRandomVal(), algo, w, h, sz, data);
            break;
        case SrcChoice::Constant: {
            char makeFullInt[] = {(char)constantVal, (char)constantVal, (char)constantVal, (char)constantVal};
            processArray<int>(*reinterpret_cast<int*>(&makeFullInt[0]), algo, w, h, sz, data);
            break;
        }
        case SrcChoice::Image: {
            processArray<NativelyCachedBmp*>(resizedDataSrcBmp, algo, w, h, sz, data);
            break;
        }
        case SrcChoice ::Text: {
            processArray<const char*>(glitchAlgo.textVal.getData(), algo, w, h, sz, data);
            break;
        }

    }
}


extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_processCachedBmp)(JNIEnv* env, jobject thiz, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    if(!bmp)
        return;
    //glitchAlgo.bend(bmp->getPixels(), bmp->getWidth(), bmp->getHeight(), bmp->getLength());
    if(glitchAlgo.backgroundThread)
        if(waitForCompletion(glitchAlgo.backgroundThread, nullptr))
            getCSAPP()->fatalError("Background glitch task is deadlocked, please report this error.");
    if(glitchAlgo.algo == Algos::ALGO_PIXELSORT)
    {
        using cloture::util::string::String_t;
        String_t msg = (String_t("Pixel sorting took ") +
        PixelSorter::timeTaken) + String_t(" milliseconds. Hits = ") + PixelSorter::hits_ + ". Misses = " + PixelSorter::misses_ + ".";
        if(PixelSorter::hits_ > PixelSorter::misses_)
            msg += " if statement is likely.";
        else
            msg += " if statement is unlikely";
        getCSAPP()->displayToast(msg.getData());
    }
    glitchAlgo.addToUndo(bmp);
    memcpy(bmp->getPixels(), glitchAlgo.backgroundResult->getPixels(), bmp->getLength() * sizeof(int));

    glitchAlgo.canUseLastResult = true;
}
extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_processCachedBmpUnthreaded)(JNIEnv* env, jobject thiz, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    if(!bmp)
        return;
    glitchAlgo.bend(bmp->getPixels(), bmp->getWidth(), bmp->getHeight(), bmp->getLength());
}
unsigned GlitchAlgo::getRandomVal() {
    return rand();
}

void GlitchAlgo::processFrameData(cloture::util::common::uint8* buff, int w, int h, size_t buffsize) {
    int* iBuff = reinterpret_cast<int*>(buff);
    size_t iSize = buffsize / sizeof(int);
    bend(iBuff, w, h, iSize);
}
template<bool preserve>
static void* backgroundTaskPixel(void* ud) {
    NativelyCachedBmp* bmp = reinterpret_cast<NativelyCachedBmp*>(ud);

    NativelyCachedBmp* result = new NativelyCachedBmp(bmp);
    static_if(preserve){
        common::uint8 *alphas = cs::allocate<common::uint8>(result->getLength());
        {
            const Color_u *colors = reinterpret_cast<Color_u *>(result->getPixels());
            unsigned length = result->getLength();
            for (unsigned i = 0; i < length; ++i) {
                alphas[i] = colors[i].col.a;
            }
        }
        glitchAlgo.bend(result->getPixels(), result->getWidth(), result->getHeight(),
                        result->getLength());
        {
            Color_u *colors = reinterpret_cast<Color_u *>(result->getPixels());
            unsigned length = result->getLength();
            for (unsigned i = 0; i < length; ++i) {
                colors[i].col.a = alphas[i];
            }
        }
        cs::deallocate(alphas);
    } static_else(preserve) {
        glitchAlgo.bend(result->getPixels(), result->getWidth(), result->getHeight(),
                        result->getLength());
    }
    glitchAlgo.backgroundThread = 0;
    glitchAlgo.backgroundResult = result;
    return nullptr; //thread ends
}
static void* backgroundTaskRaw(void* ud) {
    if(ud) {
        FILE *rawfile = (FILE *) ud;
        fseek(rawfile, 0L, SEEK_END);
        size_t sz = ftell(rawfile) - 14L;
        fseek(rawfile, 14L, SEEK_SET);
        char *data = cs::allocate<char>(sz + 4);
        memset(data, 0, sz + 4);
        fread(data, 1, sz, rawfile);
        glitchAlgo.bend((int *) data, 0, 0, sz / 4);
        fseek(rawfile, 14L, SEEK_SET);
        fwrite(data, 1, sz, rawfile);

        cs::deallocate(data);
        fclose(rawfile);
        NativelyCachedBmp *result = cs::BitmapUtils::decodeImageFile(*glitchAlgo.rawPath);
        glitchAlgo.currentRaw = fopen(glitchAlgo.rawPath->getData(), "r+");
        glitchAlgo.backgroundResult = result;
    }
    glitchAlgo.backgroundThread = 0;
    return nullptr;
}
void GlitchAlgo::backgroundPreprocessAndCache() {

    if(!currentUnresizedCopy || algo == Algos::None)
        return;
    if(backgroundThread) {
        if(waitForCompletion(backgroundThread, nullptr))
            getCSAPP()->fatalError("Background glitch task is deadlocked, please report this error.");

        backgroundThread = 0;
        if(backgroundResult) {
            delete backgroundResult;
            backgroundResult = nullptr;
        }
    }
    if(backgroundResult) {
        delete backgroundResult;
        backgroundResult = nullptr;
    }
    pthread_t newthread;
    if(datamode == DataMode::Pixel) {
        if (pthread_create(&newthread, nullptr, preserveTransparency ? backgroundTaskPixel<true> : backgroundTaskPixel<false>, currentUnresizedCopy))
            getCSAPP()->fatalError("Failed to create background glitch task.");
    }
    else if(!rawModeDisabled&& datamode == DataMode::Raw) {
            if (pthread_create(&newthread, nullptr, backgroundTaskRaw, currentRaw))
                getCSAPP()->fatalError("Failed to create background glitch task.");
        }
    else {
            getCSAPP()->fatalError("Invalid datamode.");
        }
    backgroundThread = newthread;
}

unsigned unifyBits(NativelyCachedBmp* bmp, unsigned length, unsigned i = 0 ) {
    unsigned end = bmp->getLength();
    unsigned result = 0;
    for(unsigned i = 0; i < end; ++i)
        result |= bmp->getPixels()[i];
    return result;
}

unsigned unifyBits(NativelyCachedBmp* bmp) {
    return unifyBits(bmp, bmp->getLength());
}

unsigned lowestVal(NativelyCachedBmp* bmp, unsigned length, unsigned i = 0) {
    unsigned result = -1;
    for(; i < length; ++i) {
        if(static_cast<unsigned>(bmp->getPixels()[i]) < result)
            result = bmp->getPixels()[i];
    }
    return result;
}

unsigned lowestVal(NativelyCachedBmp* bmp) {
    return lowestVal(bmp, bmp->getLength());
}

unsigned commonBits(NativelyCachedBmp* bmp, unsigned length, unsigned i = 0) {
    unsigned end = bmp->getLength();
    unsigned result = -1;
    for(unsigned i = 0; i < end; ++i)
        result &= bmp->getPixels()[i];
    return result;
}

unsigned commonBits(NativelyCachedBmp* bmp) {
    return commonBits(bmp, bmp->getLength());
}

void subtractLowerBound(NativelyCachedBmp* bmp, unsigned bound, unsigned length, unsigned i = 0) {
    unsigned end = bmp->getLength();
    for(unsigned i = 0; i < end; ++i)
        bmp->getPixels()[i] -= bound;
}

void subtractLowerBound(NativelyCachedBmp* bmp, unsigned bound) {
    subtractLowerBound(bmp, bound, bmp->getLength());
}

void GlitchAlgo::notifyBitmapChange(NativelyCachedBmp *bmp) {

    if(currentUnresizedCopy) {
        if(backgroundThread) {
            if(waitForCompletion(backgroundThread, nullptr))
                getCSAPP()->fatalError("Background glitch task is deadlocked, please report this error.");
            if(backgroundResult) {
                delete backgroundResult;
                backgroundResult = nullptr;
            }
        }
        backgroundThread = 0;
        delete currentUnresizedCopy;
        currentUnresizedCopy = nullptr;

    }
    if(backgroundResult) {
        delete backgroundResult;
        backgroundResult = nullptr;
    }
    if(!bmp)
        return;

    NativelyCachedBmp* compress = new NativelyCachedBmp(bmp);
    unsigned unified = unifyBits(compress);
    unsigned l = bmp->getLength();
    unsigned unifyHalf = unifyBits(bmp, l / 2);
    unsigned unifyOtherHalf = unifyBits(bmp, l, l / 2);

    unsigned common = commonBits(compress);
    unsigned commonHalf = commonBits(compress, l/2 );
    unsigned commonOtherHalf = commonBits(compress, l, l/2);
    unsigned lowest = lowestVal(compress);
    unsigned lowestHalf = lowestVal(compress, l/2);
    unsigned lowestOtherHalf = lowestVal(compress, l, l/2);
    subtractLowerBound(compress, lowest - 1);

    unsigned unified2 = unifyBits(compress);

    delete compress;



    currentUnresizedCopy = new NativelyCachedBmp(bmp);

}

extern "C"
JNIEXPORT void JNICALL makeJniName(BendingView_notifyBitmapChange)(JNIEnv* env, jclass viewclass, jlong ptr) {
    glitchAlgo.notifyBitmapChange(NativelyCachedBmp::decodeLongPtr(ptr));
}

extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_startBackgroundTask)(JNIEnv* env, jclass viewclass) {
    glitchAlgo.backgroundPreprocessAndCache();
}

void GlitchAlgo::openRaw(cloture::util::string::String_t filepath) {
    if(currentRaw)
        fclose(currentRaw);
    if(rawPath)
        delete rawPath;
    currentRaw = fopen(filepath.getData(), "r+");
    rawPath = new cloture::util::string::String_t(filepath);
}

extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_notifyRawFile)(JNIEnv* env, jclass viewclass, jstring s) {
    glitchAlgo.openRaw(getCSAPP()->jstringToUTF(s));
}

void GlitchAlgo::setPreserveTransparency(bool b) {
    preserveTransparency = b;
    app->prefsPutInt("preserve_transparency", b);
}
bool GlitchAlgo::getPreserveTransparency() {
    return preserveTransparency;
}

glitchalgo_callgettertype(PreserveTransparency, preserveTransparency, jboolean)
glitchalgo_callsettertype(PreserveTransparency, preserveTransparency, jboolean)

void GlitchAlgo::addToUndo(NativelyCachedBmp *newUndo, bool clear_redo) {
    if(undoIndex < undoMax)
        undoHistory[undoIndex++] = new NativelyCachedBmp(newUndo);
    else
    {
        delete undoHistory[0];
        cs::copyMemory(&undoHistory[0], &undoHistory[1], undoMax-1);
        undoHistory[undoIndex] = new NativelyCachedBmp(newUndo);
    }
    if(clear_redo)
        clearRedoHistory();
}

void GlitchAlgo::addToRedo(NativelyCachedBmp *newRedo) {
    if(redoIndex < redoMax)
        redoHistory[redoIndex++] = new NativelyCachedBmp(newRedo);
    else
    {
        delete redoHistory[0];
        cs::copyMemory(&redoHistory[0], &redoHistory[1], redoMax-1);
        redoHistory[redoIndex] = new NativelyCachedBmp(newRedo);
    }
}

void GlitchAlgo::clearUndoHistory() {
    if(!undoIndex)
        return;
    for(unsigned i = 0; i < undoIndex; ++i) {
        if(undoHistory[i]){
            delete undoHistory[i];
            undoHistory[i] = nullptr;
        }
    }
    undoIndex = 0;
}


void GlitchAlgo::clearRedoHistory() {
    if(!redoIndex)
        return;
    for(unsigned i = 0; i < redoIndex; ++i) {
        if(redoHistory[i]){
            delete redoHistory[i];
            redoHistory[i] = nullptr;
        }
    }
    redoIndex = 0;
}

NativelyCachedBmp* GlitchAlgo::popUndo() {
    if(!undoIndex)
        return nullptr;
    NativelyCachedBmp** resultptr = &undoHistory[--undoIndex];
    NativelyCachedBmp* result = *resultptr;
    *resultptr = nullptr;
    return result;
}
NativelyCachedBmp* GlitchAlgo::popRedo() {
    if(!redoIndex)
        return nullptr;
    NativelyCachedBmp** resultptr = &redoHistory[--redoIndex];
    NativelyCachedBmp* result = *resultptr;
    *resultptr = nullptr;
    return result;
}
extern "C"
JNIEXPORT jboolean JNICALL makeJniName(GlitchAlgo_undoGlitch)(JNIEnv* env, jclass glitchclass, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    if(!bmp)
        return JNI_FALSE;
    NativelyCachedBmp* undo = glitchAlgo.popUndo();
    if(!undo)
        return JNI_FALSE;
    if(bmp->getLength() != undo->getLength()) {
        glitchAlgo.clearUndoHistory();
        return JNI_FALSE;
    }
    glitchAlgo.addToRedo(bmp);
    cs::copyMemory(bmp->getPixels(), undo->getPixels(), bmp->getLength());
    delete undo;
    return JNI_TRUE;

}
extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_clearUndo)(JNIEnv* env, jclass glitchclass) {
    glitchAlgo.clearUndoHistory();
}

extern "C"
JNIEXPORT jboolean JNICALL makeJniName(GlitchAlgo_redoGlitch)(JNIEnv* env, jclass glitchclass, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    if(!bmp)
        return JNI_FALSE;
    NativelyCachedBmp* redo = glitchAlgo.popRedo();
    if(!redo)
        return JNI_FALSE;
    if(bmp->getLength() != redo->getLength()) {
        glitchAlgo.clearRedoHistory();
        return JNI_FALSE;
    }
    glitchAlgo.addToUndo(bmp, false);
    cs::copyMemory(bmp->getPixels(), redo->getPixels(), bmp->getLength());
    delete redo;
    return JNI_TRUE;

}
extern "C"
JNIEXPORT void JNICALL makeJniName(GlitchAlgo_clearRedo)(JNIEnv* env, jclass glitchclass) {
    glitchAlgo.clearRedoHistory();
}