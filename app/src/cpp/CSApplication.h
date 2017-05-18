//
// Created by chris on 1/8/17.
//

#ifndef IMAGEBENDER_APPLICATION_H
#define IMAGEBENDER_APPLICATION_H

#include <NativelyCachedBmp.h>
#include "JavaClass.h"
constexpr jint jniVersion = JNI_VERSION_1_4;
class JClassLoader;
class JavaClass;
class CSPrefs;
class CSApplication {
    friend class CSPrefs;
    JavaVM* vm;
    JavaClass* mainActivityClass;
    JavaClass* csAppClass;
    JavaClass* sharedPrefsClass;
    JavaClass* sharedPrefsEditorClass;
    JavaClass* windowManagerClass;
    JavaClass* pointClass;
    JavaClass* displayClass;
    JavaClass* fileClass;
    JavaClass* applicationClass;
    JavaClass* stringClass;
    JavaClass* contextClass;
    JavaClass* threadClass;
    JavaClass* activityClass;
    JavaClass* nativeDlgClass;
    jmethodID threadInterruptedID;
    jmethodID makeNativeDlgID;
    JClassLoader* classLoader;
    CSPrefs* prefs;

    jobject mainActivityObject;
    jobject windowManagerObject;
    jobject defaultDisplay;
    jobject applicationObject;
    jobject applicationContext;

    int screenWidth, screenHeight;
    cloture::util::string::String_t cacheDir;

    volatile pid_t owner = 0;

    bool acquire();
    bool release();

public:
    jobject currentContext;
    CSApplication(JavaVM* vm);
    void registerMainActivity(JNIEnv* env, jobject mainActivity);
    void makeEnvCurrent(JNIEnv* env);
    void initClasses();
    JNIEnv* operator ->() {
        return getEnv();
    }
    JNIEnv* getEnv() {
        JNIEnv* env;
        getVM()->GetEnv((void**)&env, jniVersion);
        return env;
    }

    inline JClassLoader* getClassLoader() {
        return classLoader;
    }
    constexpr JavaVM* getVM() {
        return vm;
    }
    __forceinline CSPrefs& getPrefs() {
        return *prefs;
    }

    const cloture::util::string::String_t& getCacheDir() const;

    void updateEnv(JNIEnv*);

    void displayToast(const char* msg);
    bool isThreadInterrupted(jobject threadObj);

    cloture::util::string::String_t jstringToUTF(jstring s);
    //void releaseUTF(jstring s, const char* u);

    using dialogcb_t = void (*)(cloture::util::string::String_t*);
    struct NativeDialog_t {
        dialogcb_t cb;
        const char* hint;
        const char* title;
        const char* pos;
        const char* neg;
        bool runOnThread;
    };

    void showNativeDialog(NativeDialog_t& dlg);
    __noreturn static void fatalError(const char* msg);

    NativelyCachedBmp* resizeLoadedBitmap(NativelyCachedBmp* bmp);
    static long long getMilliseconds();
    int prefsGetInt(const char*);
    void prefsPutInt(const char*, int);
    const char* getTag() const;
    static cloture::util::string::String_t getApkDir();
};

extern CSApplication *__restrict csApp;

CSApplication* getCSAPP();

#endif //IMAGEBENDER_APPLICATION_H
