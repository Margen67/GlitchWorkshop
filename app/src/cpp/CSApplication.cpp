//
// Created by chris on 1/8/17.
//

#include <util/stl.hpp>
#include <jni.h>
#include <glitch-common.h>
//#include <string>
#include "JavaClass.h"

#include "CSApplication.h"
//#include "JavaClass.h"

#include <jniHelpers.h>
#include "JClassLoader.h"
#include "CSPrefs.h"



#include "classnameCompression.h"

#include "JNIUtils.h"
#include "NativelyCachedBmp.h"
#include "BitmapUtils.h"
#include "SharedLibrary.h"
#include "AndroidCl.h"
using cloture::util::string::String_t;
using namespace cs;

constexpr const char* packageName = "com/example/chris/imagebender";

CSApplication * RESTRICT csApp;

CSApplication* getCSAPP() {
    return csApp;
}

const char* CSApplication::getTag() const {
    return "GltchWrk";
}




extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    BitmapUtils::initLibAV();
    csApp = new CSApplication(vm);
    return jniVersion;
}

void CSApplication::makeEnvCurrent(JNIEnv *env) {
    //cachedEnv = env;
}

CSApplication::CSApplication(JavaVM *vm) {
    this->vm = vm;
    JNIEnv* env;
    vm->GetEnv((void**)&env, jniVersion);
    makeEnvCurrent(env);
    classLoader = new JClassLoader(*this);
}
//dbgFunctionMacro(COMPRESS_CLASSNAME("android/content/SharedPreferences"));
constexpr auto sharedPrefsName = COMPRESS_CLASSNAME("android/content/SharedPreferences");


void CSApplication::initClasses() {

    char testing[512] = {};
    compressme(&sharedPrefsName.data[0], &testing[0], sharedPrefsName.length_);

    String_t mainActivityName(packageName);
    JNIEnv* env = getEnv();
    mainActivityName.append("/MainActivity");//= String_t(packageName) + "/MainActivity";

    mainActivityClass = new JavaClass(*this, mainActivityName);

    csAppClass = new JavaClass(*this, String_t(packageName) + "/CSApp");


    //constexpr size_t sharedPrefsLength = sharedPrefsName.length();
    const char* sharedPrefsUncompressed = uncompressClassname(&sharedPrefsName.data[0]/*(const char*)sharedPrefsName*/, sharedPrefsName.length_);

    String_t sharedPrefsName(sharedPrefsUncompressed);//"android/content/SharedPreferences");
    sharedPrefsClass = new JavaClass(*this, sharedPrefsName);
    sharedPrefsEditorClass = new JavaClass(*this, sharedPrefsName.append("$Editor"));

    pointClass = new JavaClass(*this, "android/graphics/Point");
    displayClass = new JavaClass(*this, "android/view/Display");
    windowManagerClass = new JavaClass(*this, "android/view/WindowManager");
    fileClass = new JavaClass(*this, "java/io/File");
    applicationClass = new JavaClass(*this, "android/app/Application");
    stringClass = new JavaClass(*this, "java/lang/String");
    contextClass = new JavaClass(*this, "android/content/Context");
    threadClass = new JavaClass(*this, "java/lang/Thread");
    nativeDlgClass = new JavaClass(*this, String_t(packageName) + "/NativeDialog");
    activityClass = new JavaClass(*this, "android/app/Activity");
    threadInterruptedID = threadClass->getMethod(env, "isInterrupted", 'Z');//getEnv()->GetStaticMethodID(threadClass->getClass(), "isInterrupted", "()Z");
    makeNativeDlgID = nativeDlgClass->getStaticMethod(env, "makeNativeDlg",
                                                      'V', contextClass, 'J', stringClass, stringClass, stringClass, stringClass, 'Z');

    jmethodID getAppContextID = contextClass->getMethod(env, "getApplicationContext", contextClass);

    applicationContext = JNIUtils::callMethod<jobject>(env, mainActivityObject, getAppContextID);

    prefs = new CSPrefs(*this);
    cs::AndroidCl* cl = new AndroidCl();

    delete cl;

}


void CSApplication::registerMainActivity(JNIEnv* env, jobject mainActivity) {
    makeEnvCurrent(env);
    mainActivityObject = mainActivity;
    initClasses();

    jmethodID getWindowManagerID = mainActivityClass->getMethod(env, "getWindowManager", windowManagerClass);

    jobject windowManager = JNIUtils::callMethod<jobject>(env, mainActivityObject, getWindowManagerID);//env->CallObjectMethod(mainActivityObject, getWindowManagerID);


    windowManagerObject = env->NewGlobalRef(windowManager);
    jmethodID pointConstructorID = pointClass->getMethod(env, "<init>", 'V');
    jobject tempPoint = env->NewObject(pointClass->getClass(), pointConstructorID);

    jmethodID getDefaultDisplayID = windowManagerClass->getMethod(env, "getDefaultDisplay", displayClass);
    defaultDisplay = JNIUtils::callMethod<jobject>(env, windowManager, getDefaultDisplayID);

    JNIUtils::callMethod<void>(env, defaultDisplay, displayClass->getMethod(env, "getSize", 'V', pointClass), tempPoint);
    screenWidth = env->GetIntField(tempPoint, env->GetFieldID(pointClass->getClass(), "x", "I"));
    screenHeight = env->GetIntField(tempPoint, env->GetFieldID(pointClass->getClass(), "y", "I"));

    applicationObject = JNIUtils::callMethod<jobject>(env, mainActivity, mainActivityClass->getMethod(env, "getApplication", applicationClass));


    applicationObject = env->NewGlobalRef(applicationObject);

    jobject cache_dir = JNIUtils::callMethod<jobject>(env, applicationObject, applicationClass->getMethod(env, "getCacheDir", fileClass));

    jstring cache_path = (jstring)JNIUtils::callMethod<jobject>(env, cache_dir, fileClass->getMethod(env, "getPath", stringClass));
    const char* path_string = env->GetStringUTFChars(cache_path, nullptr);
    cacheDir.setData(path_string);
    cacheDir += "/";
    env->ReleaseStringUTFChars(cache_path, path_string);


}

void CSApplication::updateEnv(JNIEnv *env) {
    //cachedEnv = env;

}

void CSApplication::displayToast(const char* msg) {
    JNIEnv* env = getEnv();
    jfieldID csAppFieldID = env->GetStaticFieldID(mainActivityClass->getClass(), "csApp", csAppClass->getSignature().data);
    jobject csapp = env->GetStaticObjectField(mainActivityClass->getClass(), csAppFieldID);
    jmethodID threadedToastId = csAppClass->getMethod(env, "threadedToast", 'V', stringClass);
    jstring j_msg = env->NewStringUTF(msg);
    env->CallVoidMethod(csapp, threadedToastId, j_msg);
}

String_t CSApplication::jstringToUTF(jstring s) {
    JNIEnv* env = getEnv();
    const char* s_ = env->GetStringUTFChars(s, nullptr);
    String_t result = String_t(s_);
    env->ReleaseStringUTFChars(s, s_);
    return result;
}

const String_t& CSApplication::getCacheDir() const {
    return cacheDir;
}



extern "C"
JNIEXPORT void JNICALL makeJniName(MainActivity_registerToNative)(JNIEnv* env, jobject mainActivity) {
    csApp->registerMainActivity(env, mainActivity);
}
#include <sys/syscall.h>

bool CSApplication::acquire() {
    pid_t tid = syscall(/*SYS_gettid*/ 127);
    if(owner == 0 || owner == tid) {
        owner = tid;
        return true;
    }
    return false;
}
bool CSApplication::release() {
    if(owner == syscall(127) || owner == 0) {
        owner = 0;
        return true;
    }
    return false;
}

bool CSApplication::isThreadInterrupted(jobject threadObject) {
    JNIEnv* env = getEnv();
    jboolean result = env->CallBooleanMethod(threadObject, threadInterruptedID);
    return bool(result);
}

extern "C"
JNIEXPORT void JNICALL makeJniName(NativeDialog_donative)(JNIEnv* env, jclass nativedlgclass, jstring input, jlong call) {
    auto call_ = reinterpret_cast<CSApplication::dialogcb_t>(call);
    if(input){
        call_(new String_t(csApp->jstringToUTF(input)));
    } else {
        call_(nullptr);
    }
}

extern "C"
JNIEXPORT void JNICALL makeJniName(CSApp_notifyContextChange)(JNIEnv* env, jclass appclass, jobject ctx) {
    csApp->currentContext = env->NewGlobalRef(ctx);
}

void CSApplication::showNativeDialog(NativeDialog_t& dlg) {
    JNIEnv* env = getEnv();
    jstring hint = env->NewStringUTF(dlg.hint);
    jstring title = env->NewStringUTF(dlg.title);
    jstring pos = env->NewStringUTF(dlg.pos);
    jstring neg = env->NewStringUTF(dlg.neg);
    jlong onresult = reinterpret_cast<jlong>(dlg.cb);
    jboolean runonthread = (jboolean)dlg.runOnThread;
    JNIUtils::callMethod<void>(env, nativeDlgClass, makeNativeDlgID, currentContext, onresult, hint, title, pos, neg, runonthread);
}

__noreturn void CSApplication::fatalError(const char* msg) {
    String_t errMsg = String_t("FATAL ERROR: ") + msg;

    JNIEnv* env = csApp->getEnv();
    csApp->displayToast(errMsg.getData());

    jmethodID killID = csApp->csAppClass->getStaticMethod(env, "kill", 'V');
    JNIUtils::callMethod<void>(env, csApp->csAppClass, killID);
    while(1)
        ;
    unreachable();
}

NativelyCachedBmp* CSApplication::resizeLoadedBitmap(NativelyCachedBmp *bmp) {
    if(!bmp)
        fatalError("Cannot resize null bitmap.");
    int imgMaxWidth = BitmapUtils::imgMaxWidth > screenWidth ? screenWidth : BitmapUtils::imgMaxWidth;
    int imgMaxHeight = BitmapUtils::imgMaxHeight > screenHeight ? screenHeight : BitmapUtils::imgMaxHeight;
    if(bmp->getWidth() > imgMaxWidth || bmp->getHeight() > imgMaxHeight) {
        int newWidth, newHeight;
        bmp->calculateNewDims(imgMaxWidth, imgMaxHeight, &newWidth, &newHeight);
        return BitmapUtils::rescaleImage(bmp, newWidth, newHeight);
    } else {
        return new NativelyCachedBmp(bmp);
    }
}

extern "C"
JNIEXPORT jlong JNICALL makeJniName(MainActivity_nativeResizeLoadedBitmap)(JNIEnv* env, jclass unusedarg, jlong ptr) {
    NativelyCachedBmp* bmp = NativelyCachedBmp::decodeLongPtr(ptr);
    jlong result = NativelyCachedBmp::encodeLongPtr(csApp->resizeLoadedBitmap(bmp));
    return result;
}
#include <sys/time.h>
long long CSApplication::getMilliseconds() {
    struct timeval te;
    gettimeofday(&te, nullptr); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

int CSApplication::prefsGetInt(const char* s) {
    return getPrefs().getInt(*this, s);
}

void CSApplication::prefsPutInt(const char *s, int i) {
    getPrefs().putInt(*this, s, i);
}

String_t CSApplication::getApkDir() {
    return String_t("/data/data/com.example.chris.imagebender/");
}