//
// Created by chris on 12/18/16.
//

#include "glitch-common.h"
#include "jni.h"
#include "jniHelpers.h"

extern "C"
JNIEXPORT jlong JNICALL makeJniName(CSApp_generateInitialSeed) (JNIEnv* env, jobject thiz) {
    jclass systemClockClass = env->FindClass("android/os/SystemClock");
    jmethodID milliId = env->GetStaticMethodID(systemClockClass, "currentThreadTimeMillis", "()J");
    jlong millisecs = env->CallStaticLongMethod(systemClockClass, milliId);
    void* p = cs::allocate<void>(1);
    jlong pval = reinterpret_cast<int>(p);
    cs::deallocate(p);
    float pvalAsFloat = (float)pval;

    jlong result = (millisecs * *reinterpret_cast<int*>(&pvalAsFloat)) + (reinterpret_cast<int>(systemClockClass)) ^ (~reinterpret_cast<int>(env));
    srand((unsigned int)result);
    return result;
}