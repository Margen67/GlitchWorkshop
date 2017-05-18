//
// Created by chris on 1/9/17.
//
#include <util/stl.hpp>
#include <jni.h>
#include <glitch-common.h>
#include <string>
#include "JavaClass.h"
#include "CSPrefs.h"


CSPrefs::CSPrefs(CSApplication &app) : preferencesClass(*app.sharedPrefsClass), editorClass(*app.sharedPrefsEditorClass){
    JavaClass preferenceManagerClass(app, "android/preference/PreferenceManager");
    jmethodID p = preferenceManagerClass.getStaticMethod(app.getEnv(), "getDefaultSharedPreferences", &preferencesClass, app.contextClass);
    sharedPrefs = app->NewGlobalRef(app->CallStaticObjectMethod(preferenceManagerClass.getClass(), p, /*app.mainActivityObject*/app.applicationContext));

    editMethod = preferencesClass.getMethod(app.getEnv(), "edit", &editorClass);
    getIntMethod = preferencesClass.getMethod(app.getEnv(), "getInt", 'I', app.stringClass, 'I');
    putIntMethod = editorClass.getMethod(app.getEnv(), "putInt", &editorClass, app.stringClass, 'I');
    applyMethod = editorClass.getMethod(app.getEnv(), "apply", 'V');
}

int CSPrefs::getInt(CSApplication& app, const char *key) {
    jstring jkey = app->NewStringUTF(key);
    return app->CallIntMethod(sharedPrefs, getIntMethod, jkey, -1);
}

void CSPrefs::putInt(CSApplication &app, const char *key, int val) {
    jobject editor = app->CallObjectMethod(sharedPrefs, editMethod);
    app->CallObjectMethod(editor, putIntMethod, app->NewStringUTF(key), val);
    app->CallVoidMethod(editor, applyMethod);
}