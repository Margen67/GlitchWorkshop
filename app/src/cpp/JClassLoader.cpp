//
// Created by chris on 1/9/17.
//
#include "util/stl.hpp"
#include <jni.h>
#include <glitch-common.h>
#include <string>
#include "JavaClass.h"
#include "JClassLoader.h"
#include "CSApplication.h"

using cloture::util::string::String_t;

JClassLoader::JClassLoader(CSApplication& env) {
    classLoaderClass = new JavaClass(env, "java/lang/ClassLoader");
    jmethodID loaderCtor = env->GetStaticMethodID(classLoaderClass->getClass(), "getSystemClassLoader", "()Ljava/lang/ClassLoader;");//classLoaderClass->getMethod(env.getEnv(), "<init>", 'V');
    systemClassloader = env->NewGlobalRef(
            env->CallStaticObjectMethod(classLoaderClass->getClass(), loaderCtor)
    );
    loadClassMethod = env->GetMethodID(classLoaderClass->getClass(), "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

}

bool JClassLoader::loadClass(JNIEnv* env, const char* name) {

    size_t namelen = strlen(name);
    char* modifiedName = new char[namelen + 1];
    memset(modifiedName, 0, namelen + 1);
    memcpy(modifiedName, name, namelen);

    for(unsigned i = 0; i < namelen; ++i) {
        if(modifiedName[i] == '/')
            modifiedName[i] = '.';
    }
    jstring jname = env->NewStringUTF(modifiedName);



    env->CallObjectMethod(systemClassloader, loadClassMethod, jname);
    delete [] modifiedName;
    if(env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}