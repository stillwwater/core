#ifndef CORE_RAND_H_
#define CORE_RAND_H_

#include "core/basic.h"
#include "core/splitmix64.h"
#include "core/xoshiro256.h"

// Returns a random integer in the range min(T) <= x <= max(T). If no template
// argument is provided the result is an i32.
template <typename T = i32, typename R>
T
rand_int(R *rng)
{
    static_assert(sizeof(T) <= sizeof(u64));
    return rand_next(rng) >> (64 - (sizeof(T) * CHAR_BIT));
}

// Returns a random integer in the range r_min <= x < r_max
template <typename T = i32, typename R>
T
rand_int(R *rng, T r_min, T r_max)
{
    return (T)((u64)rand_int<T, R>(rng) % (r_max - r_min) + r_min);
}

// Returns a random float in the range 0 <= x < 1. If no template argument is
// provided, the return value is a f32. Only f32 and f64 are supported.
template <typename T = f32, typename R>
T
rand_float(R *rng)
{
    // Avoids partial template function specialization.
    static_assert(sizeof(T) == sizeof(f64) || sizeof(T) == sizeof(f32));
    if constexpr (sizeof(T) == sizeof(f64)) {
        u64 x = rand_next(rng);
        u64 y = (0x3FFull << 52) | (x >> 12);
        return *((T *)&y) - 1.0;
    }
    if constexpr (sizeof(T) == sizeof(f32)) {
        u32 x = rand_next(rng) >> 32;
        u32 y = 0x3F800000 | (x >> 9);
        return *((T *)&y) - 1.0f;
    }
}

// Returns a random float in the range r_min <= x < r_max.
template <typename T = f32, typename U, typename R>
T
rand_float(R *rng, U r_min, U r_max)
{
    return rand_float<T, R>(rng) * ((T)r_max - (T)r_min) + (T)r_min;
}

#endif // CORE_RAND_H_
