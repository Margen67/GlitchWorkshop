//
// Created by chris on 12/13/16.
//
#include "csconfig.h"
#include "util/stl.hpp"
#include "glitch-common.h"
#include "jni.h"
#include "jniHelpers.h"

#include "NativelyCachedBmp.h"
#include "CSApplication.h"
#include "BitmapUtils.h"

using namespace cs;
using cloture::util::string::String_t;

#if PARANOID == 1
    using cloture::util::vector::Vector_t;
    static Vector_t<void*> all_allocated_nativebmps;
    static Vector_t<void*> all_freed_nativebmps;
    static Vector_t<NativelyCachedBmp*> all_valid_nativebmps;
#endif


extern "C"
JNIEXPORT jlong JNICALL makeJniName(NativelyCachedBmp_cache_1bmp)(JNIEnv* env, jobject thiz, jintArray java_array, jint w, jint h) {
    NativelyCachedBmp* cachedBmp = new NativelyCachedBmp(env, java_array, w, h);
    return NativelyCachedBmp::encodeLongPtr(cachedBmp);
}

extern "C"
JNIEXPORT void JNICALL makeJniName(NativelyCachedBmp_uncache_1bmp)(JNIEnv* env, jobject thiz, jlong ptr, jintArray java_array) {
    NativelyCachedBmp* cachedBmp = NativelyCachedBmp::decodeLongPtr(ptr);
    if(cachedBmp)
        cachedBmp->uncache(env, java_array);
}

extern "C"
JNIEXPORT void JNICALL makeJniName(NativelyCachedBmp_destroy_1cached_1bmp)(JNIEnv* env, jobject thiz, jlong ptr) {
    NativelyCachedBmp* cachedBmp = NativelyCachedBmp::decodeLongPtr(ptr);
    if(cachedBmp)
        delete cachedBmp;
}

extern "C"
JNIEXPORT jlong JNICALL makeJniName(NativelyCachedBmp_duplicate_1cached_1bmp)(JNIEnv* env, jobject thiz, jlong ptr) {
    NativelyCachedBmp* src = NativelyCachedBmp::decodeLongPtr(ptr);
    if(src)
        return NativelyCachedBmp::encodeLongPtr(src->duplicate());
    return jlong(0);
}

extern "C"
JNIEXPORT jlong JNICALL makeJniName(NativelyCachedBmp_createFromPath)(JNIEnv* env, jobject thiz, jstring str) {
    String_t pathname = getCSAPP()->jstringToUTF(str);
    NativelyCachedBmp* result = BitmapUtils::decodeImageFile(pathname);
    jclass tempclass = env->GetObjectClass(thiz);
    env->SetIntField(thiz, env->GetFieldID(tempclass, "w", "I"), result->getWidth());
    env->SetIntField(thiz, env->GetFieldID(tempclass, "h", "I"), result->getHeight());
    return NativelyCachedBmp::encodeLongPtr(result);
}

extern "C"
JNIEXPORT jlong JNICALL makeJniName(NativelyCachedBmp_rescale)(JNIEnv* env, jobject thiz, jlong ptr, jint neww, jint newh) {
    NativelyCachedBmp* result = BitmapUtils::rescaleImage(NativelyCachedBmp::decodeLongPtr(ptr), neww, newh);
    return NativelyCachedBmp::encodeLongPtr(result);
}
NativelyCachedBmp::NativelyCachedBmp(JNIEnv *env, jintArray arr, jint w, jint h) : h(h), w(w) {
    //cachedBmpList.push_back(this);
    jint* p;
    jsize sz;
    p = acquireIntArray(env, arr, sz);
    pixels = cs::allocate<int>(sz);//(int*)malloc(sz*sizeof(int));
    length = sz;
    cs::copyMemory(pixels, p, sz);
    env->ReleaseIntArrayElements(arr, p, 0);
}
NativelyCachedBmp* NativelyCachedBmp::duplicate() {
    return new NativelyCachedBmp(this);
}
NativelyCachedBmp::NativelyCachedBmp(NativelyCachedBmp* other)
        : h(other->h), w(other->w), length(other->length), pixels(cs::allocate<int>(other->length)), orientation(other->getOrientation()) {
    //cachedBmpList.push_back(this);
    cs::copyMemory(pixels, other->getPixels(), length);
    if(other->getUserDataCopyProc()) {
        userdata = other->getUserDataCopyProc() (this, other);
        setUserDataCopyProc(other->getUserDataCopyProc());
        setUserDataProc(other->getUserDataProc());
    }
}
NativelyCachedBmp::~NativelyCachedBmp() {

    if(userdataFreeProc) {
        userdataFreeProc(this, userdata);
    }
    if(pixels)
        cs::deallocate(pixels);
}
void NativelyCachedBmp::uncache(JNIEnv *env, jintArray arr) {
    jint* p;
    jsize sz;
    p = acquireIntArray(env, arr, sz);
    cs::copyMemory(p, pixels, length);
    env->ReleaseIntArrayElements(arr, p, 0);
}

void NativelyCachedBmp::calculateNewDims(int maxWidth, int maxHeight, int *outWidth,
                                         int *outHeight) {
    int width = w;
    int height = h;
    float ratioBitmap = (float) width / (float) height;
    float ratioMax = (float) maxWidth / (float) maxHeight;

    int finalWidth = maxWidth;
    int finalHeight = maxHeight;
    if (ratioMax > 1) {
        finalWidth = (int) ((float) maxHeight * ratioBitmap);
    } else {
        finalHeight = (int) ((float) maxWidth / ratioBitmap);
    }
    if(outWidth)
        *outWidth = finalWidth;
    if(outHeight)
        *outHeight = finalHeight;
}



extern "C"
JNIEXPORT jint JNICALL makeJniName(NativelyCachedBmp_nativeW)(JNIEnv* env, jclass unusedarg, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    return bmp ? bmp->getWidth() : -1;
}
extern "C"
JNIEXPORT jint JNICALL makeJniName(NativelyCachedBmp_nativeH)(JNIEnv* env, jclass unusedarg, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    return bmp ? bmp->getHeight() : -1;
}

extern "C"
JNIEXPORT jint JNICALL makeJniName(NativelyCachedBmp_get_1orientation)(JNIEnv* env, jclass unusedarg, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    return bmp->getOrientation();
}

Bmp565::Bmp565(NativelyCachedBmp *bmp) : pixels(BitmapUtils::convertToRgb565(bmp)), length(bmp->getLength()), w(bmp->getWidth()), h(bmp->getHeight()){

}