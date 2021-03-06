#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef uint8_t bool8;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

// http://www.gingerbill.org/article/defer-in-cpp.html
template<typename F>
struct __privDefer {
    F f;
    __privDefer(F f) : f(f) {}
    ~__privDefer() { f(); }
};

template<typename F>
__privDefer<F> __defer_func(F f) {
    return __privDefer<F>(f);
}

#define DO_GLUE(x, y) x##y
#define GLUE(x, y) DO_GLUE(x, y)
#define DEFER_VAR(x) GLUE(x, __COUNTER__)
#define defer(code) auto DEFER_VAR(_defer_) = __defer_func([&](){code;})
// ----------------------------------------------------

#define LOG(fmt, ...) (printf(fmt"\n", __VA_ARGS__), fflush(stdout))
#define LOG_NNL(fmt, ...) (printf(fmt, __VA_ARGS__), fflush(stdout))

// interferes with bs::min, bx::max
#ifdef max
    #undef max
#endif

#ifdef min
    #undef min
#endif

#ifndef mmin
    #define mmin(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef mmax
    #define mmax(a, b) ((a) > (b) ? (a) : (b))
#endif

template<typename T>
inline T clamp(T val, T valmin, T valmax)
{
    if(val < valmin) return valmin;
    if(val > valmax) return valmax;
    return val;
}

template<typename T>
inline void swap(T* t1, T* t2)
{
    T temp = *t1;
    *t1 = *t2;
    *t2 = temp;
}

#define arr_count(a) (sizeof(a)/sizeof(a[0]))

#include <bx/allocator.h>

inline bx::AllocatorI* getDefaultAllocator()
{
    static bx::DefaultAllocator g_allocator;
    return &g_allocator;
}

#define PHYS_UPDATE_DELTA (1.0 / 120.0)

inline u64 alignU64(u64 u, u64 a)
{
    if(a < 2) return u;
    u64 r = u % a;
    return r ? u + (a - r) : u;
}

