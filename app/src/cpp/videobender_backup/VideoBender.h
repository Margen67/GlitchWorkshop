#pragma once

#define     NEW_SAVE_IMPL   1

struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;
    /* pts of the next frame that will be generated */
    int64_t next_pts;
    AVFrame *frame;
    AVFrame *tmp_frame;
    float t, tincr, tincr2;
    struct SwsContext *sws_ctx;
    //AVAudioResampleContext *avr;
};

class VideoBender {
    static constexpr size_t OutputBufferSize = 4096;

    CSApplication& app;
    cloture::util::string::String_t videopath;
    AVFormatContext* inputFormatContext = nullptr;
    AVCodecContext* inputCodecContext = nullptr;
    AVCodec* inputCodec = nullptr;
    AVFrame* inputFrame = nullptr, *inputFrameRGB = nullptr;
    int firstVideoStream;
    size_t numBytes;
    cloture::util::common::uint8* inputBuffer = nullptr;
    SwsContext* inputSwsContext = nullptr;
    /*
    AVIOContext* outputIOContext = nullptr;
    cloture::util::common::uint8* outputBuffer = nullptr;*/
#if !NEW_SAVE_IMPL
    AVCodec* outputEncoder = nullptr;
    AVCodecContext* outputCodecContext = nullptr;
    FILE* tempOutputFile = nullptr;

    AVFrame* outputPicture = nullptr;
    AVPacket* outputPacket = nullptr;
#else
    OutputStream outputStream = {};
#endif
    GlitchAlgo& algo = glitchAlgo;



    void bendError(const char* errMsg);
    bool openOutputFile();

    void encodeFrame();
public:
    VideoBender(CSApplication& app, jstring videopath);
    ~VideoBender();
    void processFrames();
};