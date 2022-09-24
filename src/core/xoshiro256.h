#ifndef CORE_XOSHIRO256_H_
#define CORE_XOSHIRO256_H_

#include "core/basic.h"
#include "core/splitmix64.h"

#define xoshiro256_rotl(x, k) \
    (((x) << (k)) | ((x) >> (64 - (k))))

// Xoshiro256** pseudo random number generator.
// > https://prng.di.unimi.it/
struct Xoshiro256 {
    u64 state[4];
};

inline void
rand_init(Xoshiro256 *rng, u64 seed)
{
    Splitmix64 sm64;
    rand_init(&sm64, seed);
    rng->state[0] = rand_next(&sm64);
    rng->state[1] = rand_next(&sm64);
    rng->state[2] = rand_next(&sm64);
    rng->state[3] = rand_next(&sm64);
}

inline u64
rand_next(Xoshiro256 *rng)
{
    u64 *s = rng->state;
    u64 a = s[1] * 5;
    u64 result = xoshiro256_rotl(a, 7) * 9;

    u64 t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    u64 b = s[3];
    s[3] = xoshiro256_rotl(b, 45);

    return result;
}

#undef xoshiro256_rotl

#endif // CORE_XOSHIRO256_H_
