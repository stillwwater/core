#ifndef CORE_MACRO_H_
#define CORE_MACRO_H_

#define M_CAT_(a, b) a ## b
#define M_CAT(a, b) M_CAT_(a, b)

#define M_MIN(a, b) ((a) < (b) ? (a) : (b))
#define M_MAX(a, b) ((a) > (b) ? (a) : (b))

#if defined(__GNUC__) || defined(__clang__)
#define M_PRINTF_FMT(a, b) __attribute__((format(printf, a, b)))
#else
#define M_PRINTF_FMT(a, b)
#endif

#endif // CORE_MACRO_H_
