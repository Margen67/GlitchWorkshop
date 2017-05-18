
#include <util/stl.hpp>
#include <jni.h>
#include <glitch-common.h>

#include <jniHelpers.h>
#include "JavaClass.h"

//#include "CSApplication.h"

#include "FileUtils.h"

extern "C" {
    #include <libavcodec/avcodec.h>

    #include <libavutil/imgutils.h>
}

extern int ffmpeg(int argc, char** argv);
void set_frame_proc(void (*proc)(AVFrame*, AVPixelFormat));

#include "BitmapUtils.h"
#include "pixelsort.h"
#include "GlitchAlgo.h"
#include "VideoBender.h"
#include "GlobalError.h"
using cloture::util::string::String_t;
using namespace cloture::util::common;


volatile String_t* savename = nullptr;
static constexpr AVPixelFormat DEFAULT_PIXFORMAT = AV_PIX_FMT_RGB32;
static int* lastOriginal = nullptr;
static int* lastConverted = nullptr;

static void frame_bend_proc(AVFrame* frame, AVPixelFormat fmt) {
    int* converted = cs::BitmapUtils::convertFrame(frame, fmt, DEFAULT_PIXFORMAT, lastConverted);
    glitchAlgo.bend(converted, frame->width, frame->height, (frame->width*frame->height));
    AVFrame* f = av_frame_alloc();
    avpicture_fill((AVPicture*)f, (unsigned char*)converted, DEFAULT_PIXFORMAT, frame->width, frame->height);
    f->width = frame->width;
    f->height = frame->height;
    int* original = cs::BitmapUtils::convertFrame(f, DEFAULT_PIXFORMAT, fmt, lastOriginal);
    av_frame_free(&f);
    avpicture_fill((AVPicture*)frame, (unsigned char*)original, fmt, frame->width, frame->height);
    lastOriginal = original;
    lastConverted = converted;
}

#if FFMPEG_DYNAMIC
#include "SharedLibrary.h"
#endif

extern "C"
JNIEXPORT void JNICALL makeJniName(VideoProcessingThread_runNative)(JNIEnv* env, jobject thiz, jstring videopath) {

    CSApplication &app = *getCSAPP();
    auto start = CSApplication::getMilliseconds();
    String_t path = app.jstringToUTF(videopath);
    String_t output = app.getCacheDir() + "temp.avi";
    char* pathdat = strdup(path.getData());
    char* outputdat = strdup(output.getData());



    lastOriginal = lastConverted = nullptr;
    char* args[] = {
            "ffmpeg", "-i",  pathdat, outputdat //"-r",  "1/1", outputdat
    };
#if FFMPEG_DYNAMIC
    {
        cs::SharedLibrary ffmpegLib((app.getCacheDir() + "ffmpeg.so").getData());
        auto _set_frame_proc = ffmpegLib.getSymbol<decltype(&set_frame_proc)>("set_frame_proc");
        if (!_set_frame_proc)
            globalError("Woops! Version of ffmpeg loaded does not support callbacks!");

        auto _ffmpeg_main = ffmpegLib.getSymbol<decltype(&ffmpeg)>("ffmpeg");
        if (!_ffmpeg_main)
            getCSAPP()->fatalError("Couldn't locate main ffmpeg function!");
        int result = _ffmpeg_main(sizeof(args) / sizeof(args[0]), &args[0]);
        if (result)
            cs::globalError("Error occurred while glitching video! Error code: 0x%X.", result);
    }
#else
    set_frame_proc(frame_bend_proc);
    ffmpeg(sizeof(args) / sizeof(args[0]), &args[0]);
#endif
    //
    auto end = CSApplication::getMilliseconds();
    while(likely(!savename)){
        if(unlikely(app.isThreadInterrupted(thiz)))
            return;
    }
    String_t dst = (String_t("/sdcard/DCIM/") + *(String_t*)savename) + ".avi";
    //105 seconds

    cs::FileUtils::copyFile(outputdat, dst.getData());
    String_t endmessage = String_t("Saved glitched video to ") + dst + ". Glitching took " + (end - start) + " milliseconds.";
    app.displayToast(endmessage.getData());
    cs::deallocate(pathdat);
    cs::deallocate(outputdat);

    if(lastOriginal) {
        av_free(lastOriginal);
        lastOriginal = nullptr;
    }
    if(lastConverted) {
        av_free(lastConverted);
        lastConverted = nullptr;
    }
}

/*
 * without these declared you get linker errors from libavxxx
 */
extern "C" float log2f(float x) {
    return __builtin_log2f(x);
}

extern "C" double log2(double x) {
    return __builtin_log2(x);
}

extern "C"
JNIEXPORT void JNICALL makeJniName(VideoProcessingThread_notifyHaveSavename)(JNIEnv* env, jobject thiz, jstring videopath) {

    const char* s = env->GetStringUTFChars(videopath, nullptr);

    savename = new String_t(s);
    env->ReleaseStringUTFChars(videopath, s);
}