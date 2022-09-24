#include "core/memory.h"

#include <stdlib.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER

#include "core/basic.h"

void *
alignptr(void *ptr, usize align)
{
    uintptr_t mask = align - 1;
    intptr_t result = (intptr_t)ptr;
    result += (-result) & mask;
    return (void *)result;
}

u64
nextpow2(u64 n)
{
    if (n == 1) {
        return 1;
    }
#if defined(_MSC_VER)
    unsigned long out_n;
    _BitScanReverse64(&out_n, n - 1);
    return 1ull << (out_n + 1);
#elif defined(__GNUC__) || defined(__clang__)
    return 1ull << (64 - __builtin_clzl(n - 1));
#else
    u64 result = 1;
    while (result < n) {
        result <<= 1;
    }
    return result;
#endif
}

Arena *
make_arena(usize capacity, Allocator *allocator)
{
    auto arena = (Arena *)mem::alloc(sizeof(Arena) + capacity, allocator);
    if (!arena) {
        return NULL;
    }
    arena->capacity = capacity;
    arena->allocated = 0;
    return arena;
}

Arena *
make_arena(usize size, void *buffer)
{
    if (!buffer || size < sizeof(Arena)) {
        return NULL;
    }
    auto arena = (Arena *)buffer;
    arena->capacity = size - sizeof(Arena);
    arena->allocated = 0;
    return arena;
}

void *
arena_alloc(Arena *arena, usize size, usize align)
{
    u8 *block = arena->data + arena->allocated;
    u8 *ptr = (u8 *)alignptr(block + sizeof(usize), align);
    usize new_size = arena->allocated + (ptr - block) + size;
    if (new_size <= arena->capacity) {
        // Write the block size before the pointer we're returning.
        memcpy((usize *)ptr - 1, &size, sizeof(usize));
        arena->allocated = new_size;
        return ptr;
    }
    return NULL;
}

void *
arena_realloc(Arena *arena, void *block, usize size, usize align)
{
    if (!block) {
        return arena_alloc(arena, size, align);
    }
    assert((uintptr_t)block % align == 0);
    u8 *ptr = (u8 *)block;
    usize block_size;
    memcpy(&block_size, (usize *)ptr - 1, sizeof(usize));
    usize new_size = arena->allocated + ((i64)size - (i64)block_size);
    if (ptr + block_size == arena->data + arena->allocated) {
        // End of the arena, just grow or shrink the allocated space.
        if (new_size <= arena->capacity) {
            memcpy((usize *)ptr - 1, &size, sizeof(usize));
            arena->allocated = new_size;
            return ptr;
        }
        return NULL;
    }
    void *new_block = arena_alloc(arena, size, align);
    if (new_block) {
        memcpy(new_block, block, M_MIN(block_size, size));
    }
    return new_block;
}

void
arena_reset(Arena *arena)
{
    arena->allocated = 0;
}

static void *
system_alloc(Allocator *allocator, usize size, usize align)
{
    (void)allocator;
    (void)align;
    return malloc(size);
}

static void *
system_realloc(Allocator *allocator, void *block, usize size, usize align)
{
    (void)allocator;
    (void)align;
    return realloc(block, size);
}

static void
system_free(Allocator *allocator, void *block)
{
    (void)allocator;
    free(block);
}

// Global allocator.
Mallocator mallocator{{system_alloc, system_realloc, system_free}};

static void *
linear_alloc(Allocator *allocator, usize size, usize align)
{
    auto linear_allocator = static_cast<Linear_Allocator *>(allocator);
    return arena_alloc(linear_allocator->arena, size, align);
}

static void *
linear_realloc(Allocator *allocator, void *block, usize size, usize align)
{
    auto linear_allocator = static_cast<Linear_Allocator *>(allocator);
    return arena_realloc(linear_allocator->arena, block, size, align);
}

static void
linear_free(Allocator *allocator, void *block)
{
    (void)allocator;
    (void)block;
}

Linear_Allocator
make_linear_allocator(Arena *arena)
{
    return {{linear_alloc, linear_realloc, linear_free}, arena, arena->allocated};
}

void
deinit(Linear_Allocator allocator)
{
    allocator.arena->allocated = allocator.allocated_mark;
}
