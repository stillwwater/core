// Copyright 2022 stillwwater@gmail.com
//
// Permission to use, copy, modify, and/or distribute this software for
// any purpose with or without fee is hereby granted, provided that the
// above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
// WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
// AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
// PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifndef CORE_BENCHMARK_H_
#define CORE_BENCHMARK_H_

#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef BM_MIN_CYCLES
#define BM_MIN_CYCLES 2e9 // 1 second on a 2GHz cpu
#endif // BM_MIN_CYCLES

#ifndef BM_MAX_ITER
#define BM_MAX_ITER 1e9
#endif // BM_MAX_ITER

#define BM_CAT_(A, B) A ## _ ## B ## _
#define BM_CAT(A, B) BM_CAT_(A, B)

#if defined(_MSC_VER) && !defined(__llvm__) // bm_escape not defined for MSVC
#define bm_barrier() _ReadWriteBarrier()
#else
#define bm_noinline __attribute__((noinline))
#define bm_escape(ptr) __asm__ volatile("" : : "g"(ptr) : "memory")
#define bm_barrier() __asm__ volatile("" : : : "memory")
#endif

struct Benchmark {
    const char *name;
    void (*run)(int64_t iter__, Benchmark *context__);
    size_t iter;
    int64_t start, end;
    Benchmark *next;
};

extern Benchmark *bm_list;

[[maybe_unused]] static int64_t
bm_cpu_time()
{
#if defined(_MSC_VER) && defined(_M_AMD64
    return __rdtsc();
#elif defined(__amd64__) || defined(__x86_64__)
    uint64_t l, h;
    __asm__ volatile("rdtsc" : "=a"(l), "=d"(h));
    return (h << 32) | l;
#elif defined(__aarch64__)
    int64_t t;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(t));
#else
#error cpu architecture not supported.
    return 0;
#endif
}

#define benchmark(name)                                                       \
    static void BM_CAT(bm, __LINE__)(size_t iter__, Benchmark *context__);    \
    static Benchmark BM_CAT(bm_d, __LINE__)                                   \
        = {name, BM_CAT(bm, __LINE__), 1, 0, 0, bm_list};                     \
    static Benchmark *BM_CAT(bm_p, __LINE__)                                  \
        = bm_list = &BM_CAT(bm_d, __LINE__);                                  \
    static void BM_CAT(bm, __LINE__)(size_t iter__, Benchmark *context__)

#define measure for(                                                          \
    context__->start = bm_cpu_time();                                         \
    iter__ || ((context__->end = bm_cpu_time()) && false);                    \
    --iter__)

#define bm_main                                                               \
    Benchmark *bm_list;                                                       \
    bm_noinline void bm_escape_ptr(char *) {}                                 \
    int main(int argc, char *argv[])                                          \
    {                                                                         \
        int count = 0;                                                        \
        char *run_bm = argc > 1 ? argv[1] : NULL;                             \
        printf("%-20s| %-20s| it\n", "benchmark", "cy/it");                   \
        puts("--------------------|---------------------|------------");      \
        for (Benchmark *bm = bm_list; bm; bm = bm->next) {                    \
            if (!run_bm || !strcmp(run_bm, bm->name)) {                       \
                int64_t elapsed;                                              \
                for (;; bm->iter *= 10) {                                     \
                    bm->run(bm->iter, bm);                                    \
                    elapsed = bm->end - bm->start;                            \
                    if (elapsed >= BM_MIN_CYCLES || bm->iter >= BM_MAX_ITER)  \
                        break;                                                \
                }                                                             \
                printf("%-20s| %-20.0f| %zu\n",                               \
                        bm->name, elapsed / (double)bm->iter, bm->iter);      \
                ++count;                                                      \
            }                                                                 \
        }                                                                     \
        return count == 0;                                                    \
    }

#ifndef BM_HAVE_MAIN
bm_main
#endif // BM_HAVE_MAIN


#endif // CORE_BENCHMARK_H_
