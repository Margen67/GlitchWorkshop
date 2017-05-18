//
// Created by chris on 1/9/17.
//

#ifndef IMAGEBENDER_CSPREFS_H
#define IMAGEBENDER_CSPREFS_H

#include "JavaClass.h"
#include <jni.h>

class CSApplication;
class CSPrefs {
    jobject sharedPrefs;
    JavaClass preferencesClass;
    JavaClass editorClass;
    jmethodID getIntMethod;
    jmethodID putIntMethod;
    jmethodID editMethod;
    jmethodID applyMethod;
public:
    CSPrefs(CSApplication&);

    void putInt(CSApplication&, const char* key, int val);
    int getInt(CSApplication&, const char* key); //default value is -1
};


#endif //IMAGEBENDER_CSPREFS_H
