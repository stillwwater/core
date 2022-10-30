#ifndef CORE_SPLITMIX64_H_
#define CORE_SPLITMIX64_H_

#include "core/basic.h"

struct Splitmix64 {
    u64 state;
};

inline void
rand_init(Splitmix64 *rng, u64 seed)
{
    rng->state = seed;
}

inline u64
rand_next(Splitmix64 *rng)
{
    rng->state += 0x9E3779B97f4A7C15;
    u64 z = rng->state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
    return z ^ (z >> 31);
}

#endif // CORE_SPLITMIX64_H_
