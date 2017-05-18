//
// Created by chris on 1/8/17.
//
#include "util/stl.hpp"
#include <jni.h>
#include <glitch-common.h>
#include "CSApplication.h"

#include "JavaClass.h"
#include "JClassLoader.h"

char* sigBuffer = nullptr;
//using cloture::util::string::String_t;
using namespace cloture::util::string;

JavaClass::JavaClass(CSApplication& env, String_t& classpath) : classpath(classpath) {
    jclass tempclass = env->FindClass(classpath.data);
    if(env->ExceptionCheck()) {
        env->ExceptionClear();
        env.getClassLoader()->loadClass(env.getEnv(), classpath.data);
        tempclass = env->FindClass(classpath.data);
    }
    classz = (jclass)env->NewGlobalRef(tempclass);
}

JavaClass::JavaClass(class CSApplication& env, String_t&& classpath) : classpath(classpath) {
    jclass tempclass = env->FindClass(classpath.data);
    if(env->ExceptionCheck()) {
        env->ExceptionClear();
        env.getClassLoader()->loadClass(env.getEnv(), classpath.data);
        tempclass = env->FindClass(classpath.data);
    }
    classz = (jclass)env->NewGlobalRef(tempclass);
}

JavaClass::JavaClass(struct CSApplication &env, const char *classpath) : JavaClass(env, String_t(classpath)) {

}

JavaClass::JavaClass(JavaClass &other) : classz(other.getClass()), classpath(other.getClasspath()){

}

JavaClass::~JavaClass() {
    if(classz)
    {
        JNIEnv* env;
        csApp->getVM()->GetEnv((void**)&env, JNI_VERSION_1_4);
        env->DeleteGlobalRef(classz);
        classz = nullptr;
    }
}