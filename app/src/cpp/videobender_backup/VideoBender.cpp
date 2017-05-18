//
// Created by chris on 1/13/17.
//
#include <util/stl.hpp>
#include <jni.h>
#include <glitch-common.h>

#include <jniHelpers.h>
#include "JavaClass.h"

#include "CSApplication.h"

#include "FileUtils.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>

}
#include "GlitchAlgo.h"
#include "VideoBender.h"
using cloture::util::string::String_t;
using namespace cloture::util::common;

static bool didRegisterCodecs = false;

static int findFirstVideoStream(AVFormatContext* restrict fmtContext) {
    unsigned nbStreams = fmtContext->nb_streams;

    for(unsigned i = 0; i < nbStreams; ++i) {
        if(fmtContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            return i;
    }
    return -1;
}


static AVInputFormat* findFormat(const char* name) {
    AVInputFormat* first = nullptr;
    if(!strcmp(name, "mp4")) {
        while(first = av_iformat_next(first)) {
            if(!strcmp("QuickTime / MOV", first->long_name))
                return first;
        }
        return nullptr;
    }

    while(first = av_iformat_next(first)) {

        if(!strcmp(name, first->name))
                    return first;
    }
    return nullptr;
}


void VideoBender::bendError(const char *errMsg) {
    app.displayToast(errMsg);
}

bool VideoBender::openOutputFile() {
    String_t filename = app.getCacheDir() + "temp.mp4";
    tempOutputFile = fopen(filename.getData(), "wb");
    return bool(tempOutputFile);
}



VideoBender::VideoBender(CSApplication &app, jstring videopath_j) : app(app), videopath(app.jstringToUTF(videopath_j)) {

    if(!didRegisterCodecs) {
        av_register_all();
        didRegisterCodecs = true;
    }
    if(avformat_open_input(&inputFormatContext, static_cast<const char*>(videopath), nullptr, nullptr)) {
        bendError("Failed to open video file.");
        return;
    }
    if(avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
        bendError("Failed to find stream information for video.");
        return;
    }
    firstVideoStream = findFirstVideoStream(inputFormatContext);
    if( firstVideoStream == -1) {
        bendError("Couldn't find any video streams.");
        return;
    }
    inputCodecContext = avcodec_alloc_context3(nullptr);

    if(avcodec_parameters_to_context(inputCodecContext, inputFormatContext->streams[firstVideoStream]->codecpar) < 0) {
        bendError("Couldn't create codec context.");
        return;
    }
    inputCodec = avcodec_find_decoder(inputCodecContext->codec_id);
    if(!inputCodec) {
        bendError("Unsupported codec.");
        return;
    }

    if(avcodec_open2(inputCodecContext, inputCodec, nullptr) < 0) {
        bendError("Couldn't open codec!");
        return;
    }

    inputFrame = av_frame_alloc();
    inputFrameRGB = av_frame_alloc();

    if(!inputFrame || !inputFrameRGB) {
        bendError("Couldn't allocate frames!");
        return;
    }
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, inputCodecContext->width, inputCodecContext->height);
    inputBuffer = (uint8*)av_malloc(numBytes);
    inputSwsContext = sws_getContext(inputCodecContext->width,
                                            inputCodecContext->height,
                                            inputCodecContext->pix_fmt,
                                            inputCodecContext->width,
                                            inputCodecContext->height,
                                            AV_PIX_FMT_RGB24,
                                            SWS_BILINEAR, nullptr, nullptr, nullptr);

    avpicture_fill((AVPicture*)inputFrameRGB, inputBuffer, AV_PIX_FMT_RGB24, inputCodecContext->width, inputCodecContext->height);


    /*outputBuffer = (uint8*)av_malloc(VideoBender::OutputBufferSize);
    /outputIOContext = avio_alloc_context(outputBuffer, VideoBender::OutputBufferSize, 1, nullptr, nullptr, nullptr, nullptr);
    if(!outputIOContext) {
        bendError("Couldn't allocate output context!");
        return;
    }*/
    outputEncoder = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if(!outputEncoder) {
        bendError("Couldn't find output encoder!");
        return;

    }
    outputCodecContext = avcodec_alloc_context3(outputEncoder);
    if(!outputCodecContext) {
        bendError("Couldn't allocate output codec context!");
        return;
    }

    outputCodecContext->bit_rate = inputCodecContext->bit_rate;
    outputCodecContext->width = inputCodecContext->width;
    outputCodecContext->height = inputCodecContext->height;
    //outputCodecContext->time_base = inputCodecContext->time_base;
    outputCodecContext->time_base = (AVRational){1, 25};//.num = 1;
    outputCodecContext->framerate = (AVRational){25, 1};

    //outputCodecContext->time_base.den = 24;
    outputCodecContext->gop_size = inputCodecContext->gop_size;
    outputCodecContext->max_b_frames = inputCodecContext->max_b_frames;
    outputCodecContext->pix_fmt = inputCodecContext->pix_fmt;

    if(avcodec_open2(outputCodecContext, outputEncoder, nullptr) < 0) {
        bendError("Couldn't open output codec!");
        return;
    }
    if(!openOutputFile()) {
        bendError("Couldn't open temporary output file!");
        return;
    }
    outputPicture = av_frame_alloc();
    if(!outputPicture) {
        bendError("Couldn't allocate output frame!");
        return;
    }
    outputPacket = av_packet_alloc();
    if(!outputPacket) {
        bendError("Couldn't allocate output packet!");
        return;
    }
    outputPicture->format = outputCodecContext->pix_fmt;
    outputPicture->width = outputCodecContext->width;
    outputPicture->height = outputCodecContext->height;
    if(av_frame_get_buffer(outputPicture, 16) < 0) {
        bendError("Couldn't allocate frame buffer!");
        return;
    }
}

VideoBender::~VideoBender() {
    av_free(inputBuffer);
    av_free(inputFrameRGB);
    av_free(inputFrame);
    avcodec_close(inputCodecContext);
    avcodec_free_context(&inputCodecContext);
    avformat_close_input(&inputFormatContext);

}

void VideoBender::encodeFrame() {
    if(avcodec_send_frame(outputCodecContext, outputPicture) < 0) {
        bendError("Error sending frame for encoding!");
        return;
    }
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(outputCodecContext, outputPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            bendError("Error during encoding.");
            return;
        }

        //printf("encoded frame %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(outputPacket->data, 1, outputPacket->size, tempOutputFile);
        av_packet_unref(outputPacket);
    }
}

void VideoBender::processFrames() {
    int i = 0;
    AVPacket packet;
    int frameFinished;
    while(av_read_frame(inputFormatContext, &packet) >= 0) {
        if(packet.stream_index == firstVideoStream) {
            avcodec_decode_video2(inputCodecContext, inputFrame, &frameFinished, &packet);
            if(frameFinished) {
                sws_scale(inputSwsContext, (uint8 const * const *)inputFrame->data, inputFrame->linesize, 0,
                          inputCodecContext->height, inputFrameRGB->data, inputFrameRGB->linesize);
                outputPicture->pts = i;

                if(av_frame_make_writable(outputPicture) < 0) {
                    bendError("Couldn't make output frame writeable!");
                    return;
                }

                algo.processFrameData(inputBuffer, inputCodecContext->width, inputCodecContext->height, numBytes);
                if(av_image_fill_arrays(outputPicture->data, inputFrameRGB->linesize, inputBuffer, inputCodecContext->pix_fmt,
                                     inputCodecContext->width, inputCodecContext->height, 32) < 0) {
                    bendError("Failed to fill output picture arrays!");
                    return;
                }
                encodeFrame();
                i++;
            }

        }
        av_packet_unref(&packet);
    }
    fclose(tempOutputFile);
}

volatile String_t* savename = nullptr;

extern "C"
JNIEXPORT void JNICALL makeJniName(VideoProcessingThread_runNative)(JNIEnv* env, jobject thiz, jstring videopath) {
    CSApplication &app = *getCSAPP();
    app.updateEnv(env);
    VideoBender videoBender(app, videopath);
    videoBender.processFrames();
    while(!savename)
        ;

    String_t src = app.getCacheDir() + "temp.mp4";
    String_t dst = (String_t("/sdcard/DCIM/") + *(String_t*)savename) + ".mp4";
    delete savename;
    savename = nullptr;
    cs::FileUtils::copyFile(src, dst);

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