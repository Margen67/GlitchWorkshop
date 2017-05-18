//
// Created by chris on 12/13/16.
//

#ifndef IMAGEBENDER_JNIHELPERS_H
#define IMAGEBENDER_JNIHELPERS_H


static inline jint* RESTRICT acquireIntArray(JNIEnv* env, jintArray arr, jsize& out_size) {
    out_size = env->GetArrayLength(arr);
    return env->GetIntArrayElements(arr, nullptr);
}
static inline jbyte* RESTRICT acquireByteArray(JNIEnv* env, jbyteArray arr, jsize& out_size) {
    out_size = env->GetArrayLength(arr);
    return env->GetByteArrayElements(arr, nullptr);
}
#define makeJniName(name)       Java_com_example_chris_imagebender_##name



#endif //IMAGEBENDER_JNIHELPERS_H
