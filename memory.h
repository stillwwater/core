#ifndef CORE_MEMORY_H_
#define CORE_MEMORY_H_

#include "core/basic.h"

#ifndef MAX_ALIGN
// Used instead of alignof(max_align_t) because of MSVC.
#define MAX_ALIGN (2 * sizeof(void *))
#endif // MAX_ALIGN

#ifndef DEFAULT_ALLOCATOR
#define DEFAULT_ALLOCATOR mallocator
#endif // DEFAULT_ALLOCATOR

#define ALLOCATOR_PARAM(arg) Allocator *arg = &DEFAULT_ALLOCATOR

// Returns a reasonable alignment for T that is at least equivalent to
// alignof(max_align_t) to be used as the alignment parameter for allocators.
template <typename T>
constexpr usize
min_alignof()
{
    if constexpr (alignof(T) <= MAX_ALIGN) {
        return MAX_ALIGN;
    } else {
        return alignof(T);
    }
}

// Align pointer up.
void *alignptr(void *ptr, usize align);

// Returns the next power of 2 number after n.
u64 nextpow2(u64 n);

// Base allocator interface.
struct Allocator {
    void *(*alloc)(Allocator *allocator, usize size, usize align);

    // Must work when block is null.
    void *(*realloc)(Allocator *allocator, void *block, usize size, usize align);

    // Must work when block is null.
    void (*free)(Allocator *allocator, void *block);
};

struct Mallocator : Allocator {};

// The default system allocator. Ignores the alignment argument.
// Alignment used is implementation defined but usually it is 2*sizeof(void *).
extern Mallocator mallocator;

// Contiguous block of memory. Items can only be allocated at the end of the
// arena.
struct Arena {
    usize capacity;
    usize allocated;
    u8 data[];
};

// Creates a new arena by allocating new memory. The amount allocated is equal
// to `capacity` + `sizeof(Arena)`. Free with `mem::free(arena, allocator)`.
// Returns null if the allocation failed.
Arena *make_arena(usize capacity, ALLOCATOR_PARAM(allocator));

// Creates a new arena using an existing buffer. The arena capacity is equal to
// `size` - `sizeof(Arena)`.  The buffer must be at least `sizeof(Arena)` in
// size, otherwise returns null.
Arena *make_arena(usize size, void *buffer);

// Allocates a new block at the end of the arena. An extra 8 bytes is allocated
// before the block to store the block size, more bytes may be used to ensure
// proper alignment of the pointer returned. Returns null if the allocation
// failed.
void *arena_alloc(Arena *arena, usize size, usize align = MAX_ALIGN);

// Resizes a block in the arena. If the block is at the end of the arena the
// returned pointer is unchanged, otherwise a new block must be allocated and
// the contents of the existing block are copied. The pointer given to realloc
// must be a pointer returned by arena_alloc or arena_realloc and it must be
// aligned to the same alignment value as the given align argument. block may
// be null. Returns null if the allocation failed.
void *arena_realloc(Arena *arena, void *block, usize size, usize align = MAX_ALIGN);

// Deallocates the contents of the arena without freeing the underlying memory.
void arena_reset(Arena *arena);

// Wraps an arena with an allocator interface.
struct Linear_Allocator : Allocator {
    Arena *arena;
    usize allocated_mark;
};

Linear_Allocator make_linear_allocator(Arena *arena);

// Resets the arena to the state it was in before make_linear_allocator was
// called. Does not free the underlying memory used by the arena.
void deinit(Linear_Allocator allocator);

// Allocates memory for the given type. Returns NULL if the allocation failed.
template <typename T>
T *
make(ALLOCATOR_PARAM(allocator))
{
    return (T *)allocator->alloc(allocator, sizeof(T), min_alignof<T>());
}

namespace mem {

inline void *
alloc(usize size, usize align, ALLOCATOR_PARAM(allocator))
{
    return allocator->alloc(allocator, size, align);
}

inline void *
alloc(usize size, ALLOCATOR_PARAM(allocator))
{
    return allocator->alloc(allocator, size, MAX_ALIGN);
}

inline void *
realloc(void *block, usize size, usize align, ALLOCATOR_PARAM(allocator))
{
    return allocator->realloc(allocator, block, size, align);
}

inline void *
realloc(void *block, usize size, ALLOCATOR_PARAM(allocator))
{
    return allocator->realloc(allocator, block, size, MAX_ALIGN);
}

inline void
free(void *block, ALLOCATOR_PARAM(allocator))
{
    allocator->free(allocator, block);
}

} // namespace mem

#endif // CORE_MEMORY_H_
