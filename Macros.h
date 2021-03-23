#ifndef HEADER_Macros
#define HEADER_Macros

#include <assert.h> // IWYU pragma: keep
#include <limits.h>


#ifndef MINIMUM
#define MINIMUM(a, b)                  ((a) < (b) ? (a) : (b))
#endif

#ifndef MAXIMUM
#define MAXIMUM(a, b)                  ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(x, low, high)            (assert((low) <= (high)), ((x) > (high)) ? (high) : MAXIMUM(x, low))
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)                   (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef SPACESHIP_NUMBER
#define SPACESHIP_NUMBER(a, b)         (((a) > (b)) - ((a) < (b)))
#endif

#ifndef SPACESHIP_NULLSTR
#define SPACESHIP_NULLSTR(a, b)        strcmp((a) ? (a) : "", (b) ? (b) : "")
#endif

#ifndef SPACESHIP_DEFAULTSTR
#define SPACESHIP_DEFAULTSTR(a, b, s)  strcmp((a) ? (a) : (s), (b) ? (b) : (s))
#endif

#ifndef CAST_INT
//#define CAST_INT(x)               (assert((x) <= INT_MAX), assert((x) >= 0 || (uintmax_t)(-(x)) <= (uintmax_t)(-(INT_MIN))), ((int)(x)))
#define CAST_INT(x)               (assert((uintmax_t)(int)(x) == (uintmax_t)(x)), (int)(x))
#endif

#ifndef CAST_UNSIGNED
//#define CAST_UNSIGNED(x)          (assert((x) >= 0), assert((uintmax_t)(x) <= UINT_MAX), ((unsigned int)(x)))
#define CAST_UNSIGNED(x)          (assert((uintmax_t)(unsigned int)(x) == (uintmax_t)(x)), (unsigned int)(x))
#endif

#ifndef CAST_SIZET
#define CAST_SIZET(x)             (assert((x) >= 0), assert((uintmax_t)(x) <= SIZE_MAX), ((size_t)(x)))
#endif

#ifndef DNAN
#define DNAN                      ((double)NAN)
#endif

#ifdef  __GNUC__  // defined by GCC and Clang

#define ATTR_FORMAT(type, index, check) __attribute__((format (type, index, check)))
#define ATTR_NONNULL                    __attribute__((nonnull))
#define ATTR_NORETURN                   __attribute__((noreturn))
#define ATTR_UNUSED                     __attribute__((unused))
#define ATTR_ALLOC_SIZE1(a)             __attribute__((alloc_size (a)))
#define ATTR_ALLOC_SIZE2(a, b)          __attribute__((alloc_size (a, b)))
#define ATTR_MALLOC                     __attribute__((malloc))

#else /* __GNUC__ */

#define ATTR_FORMAT(type, index, check)
#define ATTR_NONNULL
#define ATTR_NORETURN
#define ATTR_UNUSED
#define ATTR_ALLOC_SIZE1(a)
#define ATTR_ALLOC_SIZE2(a, b)
#define ATTR_MALLOC

#endif /* __GNUC__ */

// ignore casts discarding const specifier, e.g.
//     const char []     ->  char * / void *
//     const char *[2]'  ->  char *const *
#if defined(__clang__)
#define IGNORE_WCASTQUAL_BEGIN  _Pragma("clang diagnostic push") \
                                _Pragma("clang diagnostic ignored \"-Wcast-qual\"")
#define IGNORE_WCASTQUAL_END    _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define IGNORE_WCASTQUAL_BEGIN  _Pragma("GCC diagnostic push") \
                                _Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#define IGNORE_WCASTQUAL_END    _Pragma("GCC diagnostic pop")
#else
#define IGNORE_WCASTQUAL_BEGIN
#define IGNORE_WCASTQUAL_END
#endif

#endif
