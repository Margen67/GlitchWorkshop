//
// Created by chris on 1/21/17.
//
#include "csconfig.h"
#include "util/stl.hpp"
#include "glitch-common.h"
#include "jni.h"
#include "jniHelpers.h"

#include "NativelyCachedBmp.h"
#include "CSApplication.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

}
#include "BitmapUtils.h"

using namespace cs;
using cloture::util::string::String_t;

static constexpr AVPixelFormat DEFAULT_PIXFORMAT = AV_PIX_FMT_RGB32;

struct BmpOrigin_t {
    AVFrame* frame;
    AVCodecID codecID;
    String_t* extension;
};

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

static void freeCachedFrame(NativelyCachedBmp* bmp, NativelyCachedBmp::ud_t encodedFrame) {
#if 0
    AVFrame* fr = reinterpret_cast<AVFrame*>(encodedFrame);
    if(fr) {
        av_frame_free(&fr);
        bmp->setUserdata(0);
    }
    bmp->setUserDataProc(nullptr);
#else
    BmpOrigin_t* fr = reinterpret_cast<BmpOrigin_t*>(encodedFrame);
    if(fr) {
        if(fr->frame)
            av_frame_free(&fr->frame);
        if(fr->extension)
            delete fr->extension;
        bmp->setUserdata(0);
    }
    bmp->setUserDataProc(nullptr);
#endif
}

static NativelyCachedBmp::ud_t copyCachedFrame(NativelyCachedBmp* me, NativelyCachedBmp* other) {
#if 0
    AVFrame* original = other->getUserdata<AVFrame*>();
    if(!original)
        CSApplication::fatalError("Null frame on cached bitmap!");

    AVFrame* result = av_frame_clone(original);
    if(!result)
        CSApplication::fatalError("Failed to clone frame!");
    return reinterpret_cast<NativelyCachedBmp::ud_t>(result);
#else
    BmpOrigin_t* original = other->getUserdata<BmpOrigin_t*>();
    if(!original)
        CSApplication::fatalError("Null frame on cached bitmap!");

    AVFrame* fr = av_frame_clone(original->frame);
    if(!fr)
        CSApplication::fatalError("Failed to clone frame!");
    BmpOrigin_t* result = new BmpOrigin_t;
    result->frame = fr;
    result->codecID = original->codecID;
    result->extension = new String_t(*original->extension);
    return reinterpret_cast<NativelyCachedBmp::ud_t>(result);
#endif
}

NativelyCachedBmp* BitmapUtils::decodeImageFile(String_t filename) {
    int orientationValue = -1;
    String_t fmt = filename.fileExtension();
    if(fmt == "jpg")
        fmt = "jpeg";
    fmt += "_pipe";

    AVInputFormat* ifmt = findFormat(fmt.getData());
    AVFormatContext *iFormatContext = nullptr;
    if (avformat_open_input(&iFormatContext,filename.getData(), ifmt, NULL)!=0)
    {
        printf("Error in opening input file %s",filename.getData());
        return nullptr;
    }
    int videoStreamIndex=-1;
    for (int a=0;a<iFormatContext->nb_streams;a++)
    {
        if (iFormatContext->streams[a]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex=a;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
        printf("Couldn't find video stream");
        avformat_close_input(&iFormatContext);
        return nullptr;
    }
    AVCodecContext *pCodecCtx=iFormatContext->streams[videoStreamIndex]->codec;
    AVCodec *pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if (!pCodec)
    {
        printf("Cannot find decoder");
        avformat_close_input(&iFormatContext);
        return nullptr;
    }
    if (avcodec_open2(pCodecCtx,pCodec, nullptr)<0)
    {
        printf("Cannot open decoder");
        avformat_close_input(&iFormatContext);
        return nullptr;
    }
    AVPacket encodedPacket;
    av_init_packet(&encodedPacket);
    encodedPacket.data = NULL;
    encodedPacket.size = 0;
//now read a frame into this AVPacket
    if (av_read_frame(iFormatContext, &encodedPacket)<0)
    {
        printf("Cannot read frame");
        av_packet_unref(&encodedPacket);
        //avcodec_close(pCodecCtx);
        avformat_close_input(&iFormatContext);
        return nullptr;
    }
    int frameFinished=0;
    AVFrame *decodedFrame=av_frame_alloc();
    avcodec_decode_video2(pCodecCtx ,decodedFrame, &frameFinished, &encodedPacket);
    AVDictionary* exif = av_frame_get_metadata(decodedFrame);
    AVDictionaryEntry* orientation = av_dict_get(exif, "Orientation", nullptr, 0);
    if(!orientation)
        orientation = av_dict_get(exif, "orientation", nullptr, 0);
    if(orientation) {
        orientationValue = atoi(orientation->value);
    }
    AVPicture* destPic = cs::allocate<AVPicture>();
//we will convert to RGB32
    //PixelFormat destFormat= PIX_FMT_RGB32;
    avpicture_alloc(destPic, DEFAULT_PIXFORMAT, decodedFrame->width,decodedFrame->height);
    SwsContext *ctxt = sws_getContext(decodedFrame->width, decodedFrame->height,
                                      (AVPixelFormat)decodedFrame->format,decodedFrame->width, decodedFrame->height,
                                      DEFAULT_PIXFORMAT, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctxt)
    {
        printf ("Error while calling sws_getContext");
        return nullptr;
    }
    sws_scale(ctxt, (const uint8_t* const*) &decodedFrame->data[0], decodedFrame->linesize, 0, decodedFrame->height, destPic->data, destPic->linesize);

    int raw_data_size = decodedFrame->height * decodedFrame->width; //each pixel is 4 bytes
    int *raw_data = cs::allocate<int>(raw_data_size);//(uint8_t*)malloc(raw_data_size);
    if(av_image_copy_to_buffer((uint8_t*)raw_data,raw_data_size*sizeof(int), (const uint8_t* const*)destPic->data, destPic->linesize,
                            DEFAULT_PIXFORMAT, decodedFrame->width,decodedFrame->height, 1) < 0)
        CSApplication::fatalError("Failed to copy decoded picture to buffer!");

    NativelyCachedBmp* result = new NativelyCachedBmp(raw_data, raw_data_size, decodedFrame->width, decodedFrame->height);

    av_packet_unref(&encodedPacket);
#if 0
    result->setUserdata(decodedFrame);
#else
    BmpOrigin_t* origin = new BmpOrigin_t;
    origin->extension = new String_t(filename.fileExtension());
    origin->codecID = pCodecCtx->codec_id;
    origin->frame = decodedFrame;
    result->setUserdata(origin);
#endif
    result->setUserDataProc(freeCachedFrame);
    result->setUserDataCopyProc(copyCachedFrame);
    result->setOrientation(orientationValue);
    //av_frame_free(&decodedFrame);

    avpicture_free(destPic);
    cs::deallocate(destPic);
    sws_freeContext(ctxt);
    avformat_close_input(&iFormatContext);
    return result;
}
NativelyCachedBmp* BitmapUtils::rescaleImage(NativelyCachedBmp* src, int neww, int newh) {
    AVFrame *srcFrame = src->getUserdata<BmpOrigin_t*>()->frame;//av_frame_alloc();
    if(av_image_fill_arrays(srcFrame->data, srcFrame->linesize, (uint8_t*)src->getPixels(), DEFAULT_PIXFORMAT, src->getWidth(), src->getHeight(), 1) < 0)
        CSApplication::fatalError("Failed to fill image arrays.");

    AVFrame* destPic = av_frame_alloc();
    int num_bytes = avpicture_get_size(DEFAULT_PIXFORMAT, neww, newh);
    uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));
    //avpicture_alloc(&destPic, AV_PIX_FMT_RGB32, neww, newh);
    avpicture_fill((AVPicture*)destPic, frame2_buffer, DEFAULT_PIXFORMAT, neww, newh);
    /*
     * srcFrame->format seems to not be the actual pixel format. using srcFrame->format leads to sws_scale failing later.
     * also, SWS_BICUBIC is SO FUCKING SLOW. with a 1.9 mb image it takes over 800 milliseconds longer to rescale
    */
    SwsContext *ctxt = sws_getContext(src->getWidth(), src->getHeight(),
                                      /*(AVPixelFormat )srcFrame->format*/DEFAULT_PIXFORMAT, neww, newh,
                                      DEFAULT_PIXFORMAT, /*SWS_BICUBIC*/SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if(!ctxt)
        CSApplication::fatalError("Couldn't allocate sws context for resizing frame!");
    sws_scale(ctxt, (const uint8_t* const *)srcFrame->data, srcFrame->linesize, 0, srcFrame->height, destPic->data, destPic->linesize);
    int *raw_data;
    int raw_data_size=newh*neww;
    raw_data = cs::allocate<int>(raw_data_size);//(uint8_t*)malloc(raw_data_size);
    if(av_image_copy_to_buffer((uint8_t*)raw_data,raw_data_size*sizeof(int), (const uint8_t* const*)destPic->data, destPic->linesize,
                            DEFAULT_PIXFORMAT, neww, newh, 1) < 0)
        CSApplication::fatalError("Failed to copy resized image to native buffer!");

    NativelyCachedBmp* result = new NativelyCachedBmp(raw_data, raw_data_size, neww, newh);
    result->setOrientation(src->getOrientation());
    result->copyFlags(src);
    sws_freeContext(ctxt);
    av_frame_free(&destPic);
    av_free(frame2_buffer);
    //av_frame_free(&srcFrame);
    return result;
}
#if 0
int WriteJPEG (AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo){
    AVCodecContext         *pOCodecCtx;
    AVCodec                *pOCodec;
    uint8_t                *Buffer;
    int                     BufSiz;
    int                     BufSizActual;
    int                     ImgFmt = PIX_FMT_YUVJ420P; //for thenewer ffmpeg version, this int to pixelformat
    FILE                   *JPEGFile;
    char                    JPEGFName[256];

    BufSiz = avpicture_get_size (
            ImgFmt,pCodecCtx->width,pCodecCtx->height );

    Buffer = (uint8_t *)malloc ( BufSiz );
    if ( Buffer == NULL )
        return ( 0 );
    memset ( Buffer, 0, BufSiz );

    pOCodecCtx = avcodec_alloc_context ( );
    if ( !pOCodecCtx ) {
        free ( Buffer );
        return ( 0 );
    }

    pOCodecCtx->bit_rate      = pCodecCtx->bit_rate;
    pOCodecCtx->width         = pCodecCtx->width;
    pOCodecCtx->height        = pCodecCtx->height;
    pOCodecCtx->pix_fmt       = ImgFmt;
    pOCodecCtx->codec_id      = CODEC_ID_MJPEG;
    pOCodecCtx->codec_type    = CODEC_TYPE_VIDEO;
    pOCodecCtx->time_base.num = pCodecCtx->time_base.num;
    pOCodecCtx->time_base.den = pCodecCtx->time_base.den;

    pOCodec = avcodec_find_encoder ( pOCodecCtx->codec_id );
    if ( !pOCodec ) {
        free ( Buffer );
        return ( 0 );
    }
    if ( avcodec_open ( pOCodecCtx, pOCodec ) < 0 ) {
        free ( Buffer );
        return ( 0 );
    }

    pOCodecCtx->mb_lmin        = pOCodecCtx->lmin =
            pOCodecCtx->qmin * FF_QP2LAMBDA;
    pOCodecCtx->mb_lmax        = pOCodecCtx->lmax =
            pOCodecCtx->qmax * FF_QP2LAMBDA;
    pOCodecCtx->flags          = CODEC_FLAG_QSCALE;
    pOCodecCtx->global_quality = pOCodecCtx->qmin * FF_QP2LAMBDA;

    pFrame->pts     = 1;
    pFrame->quality = pOCodecCtx->global_quality;
    BufSizActual = avcodec_encode_video(
            pOCodecCtx,Buffer,BufSiz,pFrame );

    sprintf ( JPEGFName, "%06d.jpg", FrameNo );
    JPEGFile = fopen ( JPEGFName, "wb" );
    fwrite ( Buffer, 1, BufSizActual, JPEGFile );
    fclose ( JPEGFile );

    avcodec_close ( pOCodecCtx );
    free ( Buffer );
    return ( BufSizActual );
}
#endif
void BitmapUtils::encodeImageFile(String_t path,
                                  String_t name, NativelyCachedBmp *bmp) {
    using cloture::util::common::uint8;
    int bufferSize = av_image_get_buffer_size(DEFAULT_PIXFORMAT, bmp->getWidth(), bmp->getHeight(), 0);

    uint8* buffer = cs::allocate<uint8>(bufferSize);
    memset(buffer, 0, bufferSize);
    BmpOrigin_t* origin = bmp->getUserdata<BmpOrigin_t*>();
    AVCodec* pOCodec = avcodec_find_encoder(origin->codecID);
    AVCodecContext* pOCodecCtx = avcodec_alloc_context3(pOCodec);

}
static bool didRegisterCodecs = false;

void BitmapUtils::initLibAV() {
    if(!didRegisterCodecs) {
        av_register_all();
        didRegisterCodecs = true;
    }
}
#define NO_FILL_BUFFER  1

int* BitmapUtils::convertFrame(AVFrame* frame, AVPixelFormat fmt, AVPixelFormat destFormat, int* outbuff) {
    //AVCodecContext
    //size_t buffsz = av_image_get_buffer_size(fmt,frame->width, frame->height, 0);
    AVFrame *destPic = av_frame_alloc();

#if !NO_FILL_BUFFER
    int num_bytes = avpicture_get_size(destFormat, frame->width, frame->height);
    uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));
#else
    uint8_t *frame2_buffer;
    if (outbuff)
        frame2_buffer = (uint8_t *) outbuff;
    else {
        int num_bytes = avpicture_get_size(destFormat, frame->width, frame->height);
        frame2_buffer = (uint8_t *) av_malloc(num_bytes * sizeof(uint8_t));
    }
#endif
    //avpicture_alloc(&destPic, AV_PIX_FMT_RGB32, neww, newh);
    avpicture_fill((AVPicture *) destPic, frame2_buffer, destFormat, frame->width, frame->height);
    /*
     * srcFrame->format seems to not be the actual pixel format. using srcFrame->format leads to sws_scale failing later.
     * also, SWS_BICUBIC is SO FUCKING SLOW. with a 1.9 mb image it takes over 800 milliseconds longer to rescale
    */
    SwsContext *ctxt = sws_getContext(frame->width, frame->height,
            /*(AVPixelFormat )srcFrame->format*/fmt, frame->width, frame->height,
                                      destFormat, /*SWS_BICUBIC*//*SWS_FAST_BILINEAR*/0, nullptr,
                                      nullptr, nullptr);
    if (!ctxt)
        CSApplication::fatalError("Couldn't allocate sws context for resizing frame!");
    sws_scale(ctxt, (const uint8_t *const *) frame->data, frame->linesize, 0, frame->height,
              destPic->data, destPic->linesize);
#if !NO_FILL_BUFFER
    int *raw_data;
    int raw_data_size=frame->height*frame->width;
    if(!outbuff)
        raw_data = (int*)av_malloc(raw_data_size*sizeof(int));//cs::allocate<int>(raw_data_size);//(uint8_t*)malloc(raw_data_size);
    else
        raw_data = outbuff;
    if(av_image_copy_to_buffer((uint8_t*)raw_data,raw_data_size*sizeof(int), (const uint8_t* const*)destPic->data, destPic->linesize,
                               destFormat, frame->width, frame->height, 1) < 0)
        CSApplication::fatalError("Failed to copy resized image to native buffer!");
#endif
    sws_freeContext(ctxt);
    av_frame_free(&destPic);
#if !NO_FILL_BUFFER
    av_free(frame2_buffer);
    return raw_data;
#else
    return (int *) frame2_buffer;
#endif
}

short * BitmapUtils::convertToRgb565(NativelyCachedBmp *bmp) {
    BmpOrigin_t* origin = bmp->getUserdata<BmpOrigin_t*>();
    return (short*)convertFrame(origin->frame, DEFAULT_PIXFORMAT, AV_PIX_FMT_RGB565);
}