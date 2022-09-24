#ifndef CORE_PANIC_H_
#define CORE_PANIC_H_

// Aborts the program.
#define panic(...) core_panic_fmt(__FILE__, __LINE__, __func__, __VA_ARGS__)

typedef void (*core_panic_proc_t)(const char *, const char *, int, const char *);

// Default panic handler.
extern core_panic_proc_t core_panic_proc;

#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline)) void
core_panic_fmt(const char *file, int ln, const char *func, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));
#elif defined(_MSC_VER)
__declspec(noinline) void
core_panic_fmt(const char *file, int ln, const char *func, const char *fmt, ...);
#else
void
core_panic_fmt(const char *file, int ln, const char *func, const char *fmt, ...);
#endif

#endif // CORE_PANIC_H_
