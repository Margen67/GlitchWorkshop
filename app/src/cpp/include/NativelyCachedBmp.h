//
// Created by chris on 12/13/16.
//

#ifndef IMAGEBENDER_NATIVELYCACHEDBMP_H
#define IMAGEBENDER_NATIVELYCACHEDBMP_H

enum class Bmpflags_e {
    UnknownOrientation = 0,

    Flags = 32
};

template<typename base, typename ud_type>
struct BmpInterface_t {
#define crtp    static_cast<base*>(this)
    using myType = BmpInterface_t<base, ud_type>;
    //using ud_t = decltype(base::userdata);
    template<typename T>
    __forceinline T getUserdata() {
        return reinterpret_cast<T>(crtp->userdata);
    }
    template<typename T>
    __forceinline void setUserdata(T value) {
        using namespace cloture::util::generic;
        if constexpr(isIntegral<T>() && isIntegral<ud_type>()) {
            crtp->userdata = static_cast<typeof(crtp->userdata)>(value);
        } else {
            crtp->userdata = reinterpret_cast<typeof(crtp->userdata)>(value);
        }
    }

    __forceinline void setUserDataProc(void (*proc)(base*, ud_type)) {
        crtp->userdataFreeProc = proc;
    }
    __forceinline auto getUserDataProc() {
        return crtp->userdataFreeProc;
    }
    __forceinline void setUserDataCopyProc(ud_type (*proc)(base* me, base* other) ) {
        crtp->userdataCopyProc = proc;
    }
    __forceinline auto getUserDataCopyProc() {
        return crtp->userdataCopyProc;
    }
};


class NativelyCachedBmp : public BmpInterface_t<NativelyCachedBmp, cloture::util::common::uint64>{
public:
    friend struct myType;
    using ud_t = cloture::util::common::uint64;
private:
    int* pixels;
    unsigned length;
    const jint w, h;
    unsigned int flags = 0;
    unsigned int orientation = -1;


public:
    ud_t userdata = 0;
    void (*userdataFreeProc)(NativelyCachedBmp*, ud_t) = nullptr;
    ud_t (*userdataCopyProc)(NativelyCachedBmp* me, NativelyCachedBmp* other) = nullptr;

    NativelyCachedBmp(JNIEnv* env, jintArray arr, jint w, jint h);
    NativelyCachedBmp(NativelyCachedBmp* other);
    __forceinline NativelyCachedBmp(int* pixels, unsigned length, int w, int h) : pixels(pixels), length(length), w(w), h(h) {}
    ~NativelyCachedBmp();
    void uncache(JNIEnv* env, jintArray arr);
    NativelyCachedBmp* duplicate();
    NativelyCachedBmp& operator=(NativelyCachedBmp& other);
    __forceinline jint getWidth() {return w;}
    __forceinline jint getHeight() {return h;}
    __forceinline int* getPixels() {return pixels;}
    __forceinline int* getPixels() volatile {return pixels;}
    __forceinline unsigned getLength() {return length;}
    __forceinline unsigned int getOrientation() const {return orientation;}
    __forceinline void setOrientation(unsigned int val) {orientation = val;}
    __forceinline bool testFlag(Bmpflags_e flag) const {
        using namespace cloture::util::generic;
        return flags & (1 << enumValue(flag));
    }
    __forceinline void setFlag(Bmpflags_e flag) {
        using namespace cloture::util::generic;
        flags |= (1 << enumValue(flag));
    }
    __forceinline void copyFlags(NativelyCachedBmp* other) {
        flags = other->flags;
    }



    void calculateNewDims(int maxWidth, int maxHeight, int* outWidth, int* outHeight);
    static __forceinline PURE NativelyCachedBmp* decodeLongPtr(jlong ptr) {
        return reinterpret_cast<NativelyCachedBmp*>(ptr);
    }
    static __forceinline PURE jlong encodeLongPtr(NativelyCachedBmp* ptr) {
        return reinterpret_cast<jlong>(ptr);
    }

    static __forceinline void* operator new(size_t size) {
        return cs::allocate<void>(size);
    }

};

class Bmp565 : public BmpInterface_t<Bmp565, NativelyCachedBmp::ud_t> {
    short* pixels;
    unsigned length;
    const jint w, h;
public:
    using ud_t = NativelyCachedBmp::ud_t;

    ud_t userdata = 0;
    void (*userdataFreeProc)(NativelyCachedBmp*, ud_t) = nullptr;
    ud_t (*userdataCopyProc)(NativelyCachedBmp* me, NativelyCachedBmp* other) = nullptr;
    Bmp565(NativelyCachedBmp*);
};


#endif //IMAGEBENDER_NATIVELYCACHEDBMP_H
