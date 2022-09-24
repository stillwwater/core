#include "core/panic.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void
core_panic(const char *msg, const char *file, int ln, const char *func)
{
    fflush(stdout);
#ifdef NDEBUG
    (void)file;
    (void)ln;
    fprintf(stderr, "panic: %s: %s\n", func, msg);
    fflush(stderr);
    abort();
#else // NDEBUG
    fprintf(stderr, "panic: %s: %s:%d: %s\n", func, file, ln, msg);
    fflush(stderr);
#ifdef _MSC_VER
    __debugbreak();
#else // _MSC_VER
    abort();
#endif // MSC_VER
#endif // NDEBUG
}

core_panic_proc_t core_panic_proc = core_panic;

void
core_panic_fmt(const char *file, int ln, const char *func, const char *fmt, ...)
{
    char msg[1024];
    va_list ap;

    va_start(ap, fmt);
    auto len = vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    if (len < (decltype(len))sizeof(msg)) {
        core_panic_proc(msg, file, ln, func);
        return;
    }

    char *msg_h = (char *)malloc(len + 1);
    if (!msg_h) {
        // Write the incomplete error message
        core_panic_proc(msg, file, ln, func);
        return;
    }
    va_start(ap, fmt);
    vsnprintf(msg_h, len + 1, fmt, ap);
    va_end(ap);
    core_panic_proc(msg_h, file, ln, func);
    free(msg_h);
}
