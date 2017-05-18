//
// Created by chris on 11/23/16.
//

#ifndef IMAGEBENDER_GLITCH_COMMON_H
#define IMAGEBENDER_GLITCH_COMMON_H


#include <cstdlib>

#define FORCEINLINE             __attribute__((always_inline))
#define RESTRICT                __restrict__
#if defined(__arm__) && 1
    #define     USE_ARM_EXTENSIONS 1
#endif

#if defined(__clang__)
    #define assume(x)   __builtin_assume(x)
#elif defined(__GNUG__)
    #define assume(x)   if(!(x))    {__builtin_unreachable();}
#endif

#if defined(__clang__) || defined(__GNUG__)
    #define     GNU_EXTENSIONS  1
#else
    #define     GNU_EXTENSIONS 0
#endif

#if defined(__clang__)
    #define aligned_ptr(value)      __attribute__((align_value(value)))
    #define choose_expr(cond, if, else)     __builtin_choose_expr(cond, if, else)
#else

    #define choose_expr(cond, if, else)     (((cond)) ? (if) : (else))
#endif

#if GNU_EXTENSIONS
    #define PURE        __attribute__ ((const))
    #define PSEUDOPURE  __attribute__((pure))
#endif



namespace cs {
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;
    using i8 = signed char;
    using i16 = signed short;
    using i32 = signed int;
    using i64 = signed long long;
    template<bool b>
    struct metabool {

    };

    template<> struct metabool<true> {
        using value = int;
    };
    template<> struct metabool<false> {
    };
};

#define static_if(expr)     __if_exists(cs::metabool<(expr)>::value)
#define static_else(expr)   __if_not_exists(cs::metabool<(expr)>::value)

#if !defined(__arm__) || !USE_ARM_EXTENSIONS
    static inline unsigned int reverseBits(unsigned int num)
    {
        unsigned int  NO_OF_BITS = sizeof(num) * 8;
        unsigned int reverse_num = 0, i, temp;

        for (i = 0; i < NO_OF_BITS; i++)
        {
            temp = (num & (1 << i));
            if(temp)
                reverse_num |= (1 << ((NO_OF_BITS - 1) - i));
        }

        return reverse_num;
    }
#endif

static PURE FORCEINLINE cs::u32 rbit(cs::u32 in) {
    #if defined(__arm__) && USE_ARM_EXTENSIONS
        cs::u32 result;

        asm("rbit %[result], %[in]"
        : [result] "+r" (result)
        : [in] "r" (in)
        );
        return result;
    #else
        return reverseBits(in);
    #endif
}
static inline int rbyte(int in) {
    return __builtin_bswap32(in);
}


union Color_u {
    unsigned val32;
    struct {
        char b, g, r, a;
    }col;
    template<typename... T>
    inline PURE Color_u(T... ts) {

        static_if(sizeof...(ts) == 1)
        {
            val32 = (... , ts);
        }
        static_if(sizeof...(ts) == 4) {
            col = {ts...};
        }
    }
};
namespace cs {
    template<typename T>
    static FORCEINLINE T* allocate(const size_t elements) {
        return (T*)memalign(16, sizeof(T) * elements);
    }
    template<typename T>
    static FORCEINLINE T* allocate() {
        return (T*)memalign(16, sizeof(T));
    }

    template<>
    FORCEINLINE void* allocate<void>(const size_t elements) {
        return memalign(16, elements);
    }

    template<typename T>
    static FORCEINLINE void deallocate(T* ptr) {
        free(ptr);
    }

    template<typename T1, typename T2>
    static FORCEINLINE void copyMemory(T1* dest, T2* src, size_t elements) {
        static_assert(sizeof(T1) == sizeof(T2));

        memcpy(dest, src, elements*sizeof(T1));
    }

    template<typename T>
    static FORCEINLINE void zeroMem(T* dest, size_t n) {
        memset(dest, 0, sizeof(T) * n);
    }
}


#endif //IMAGEBENDER_GLITCH_COMMON_H
