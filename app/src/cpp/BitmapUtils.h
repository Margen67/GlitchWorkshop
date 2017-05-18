//
// Created by chris on 1/21/17.
//

#ifndef IMAGEBENDER_BITMAPUTILS_H
#define IMAGEBENDER_BITMAPUTILS_H

namespace cs {
    namespace BitmapUtils {
        static constexpr int imgMaxHeight = 2048, imgMaxWidth = 2048;

        void initLibAV();
        NativelyCachedBmp* decodeImageFile(cloture::util::string::String_t filename);
        NativelyCachedBmp* rescaleImage(NativelyCachedBmp* src, int neww, int newh);
        void encodeImageFile(cloture::util::string::String_t path, cloture::util::string::String_t name, NativelyCachedBmp* bmp);
        //NativelyCachedBmp* fixOrientation(

        __forceinline static NativelyCachedBmp* decodeImageFile(const char* s) {
            return decodeImageFile(cloture::util::string::String_t(s));
        }
        short* convertToRgb565(NativelyCachedBmp* bmp);
        __if_not_exists(AVFrame) {
        extern "C" {
#include "libavformat/avformat.h"
        }
        }
        int* convertFrame(AVFrame* frame, AVPixelFormat fmt, AVPixelFormat destFormat, int* outBuff = nullptr);

    }

}
#endif //IMAGEBENDER_BITMAPUTILS_H
