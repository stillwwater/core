#ifndef CORE_SLICE_H_
#define CORE_SLICE_H_

#include "core/basic.h"
#include "core/memory.h"

// Slice
// * Slices do not own memory.
// * `make(N)` returns a slice to newly allocated memory.
// * Slice<void> is undefined.
// * `operator==` compares the contents of the slice, if `T` can be compared
//   using `==`.
// * Slices implicitly cast to `bool`. The expression is true if `.data` is
//   not NULL and `.count` is not zero.
template <typename T>
struct Slice {
    typedef T underlying_type;
    typedef T *iterator;

    usize count;
    T *data;

    T &
    operator[](usize i)
    {
        assert(i < count);
        return data[i];
    }

    const T &
    operator[](usize i) const
    {
        assert(i < count);
        return data[i];
    }

    operator bool() { return count != 0 && data; }
};

template <typename T>
struct Is_Slice {
    static constexpr bool value = false;
};

template <typename T>
struct Is_Slice<Slice<T>> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool
is_slice()
{
    return Is_Slice<T>::value;
}

template <typename T>
Slice(usize, T *) -> Slice<T>;

template <typename T>
typename Slice<T>::iterator
begin(Slice<T> s)
{
    return s.data;
}

template <typename T>
typename Slice<T>::iterator
end(Slice<T> s)
{
    return s.data + s.count;
}

template <typename T>
bool
operator==(Slice<T> a, Slice<T> b)
{
    if (a.count != b.count) {
        return false;
    }
    for (usize i = 0; i < a.count; ++i) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool
operator!=(Slice<T> a, Slice<T> b)
{
    return !(a == b);
}

// Allocates a new slice. The memory will be uninitialized.
template <typename T>
Slice<T>
make(usize count, ALLOCATOR_PARAM(allocator))
{
    Slice <T> s;
    s.count = count;
    s.data = (T *)mem::alloc(count * sizeof(T), min_alignof<T>(), allocator);
    return s;
}

// Allocates a new slice and copies the contents of `src`.
template <typename T>
Slice<T>
make_copy(Slice<T> src, ALLOCATOR_PARAM(allocator))
{
    Slice<T> dst = make<T>(src.count, allocator);
    memcpy(dst.data, src.data, src.count * sizeof(T));
    return dst;
}

// Returns a new slice that points to a range in the source slice starting from
// `start` until the end of the source slice.
template <typename T>
Slice<T>
slice(Slice<T> s, usize start)
{
    assert(start < s.count);
    Slice<T> result;
    result.count = s.count - start;
    result.data = s.data + start;
    return result;
}

// Returns a new slice that points to a range in the source slice. `start` and
// `end` are inclusive indecies.
template <typename T>
Slice<T>
slice(Slice<T> s, usize start, usize end)
{
    assert(start < end && end < s.count);
    Slice<T> result;
    result.count = end - start;
    result.data = s.data + start;
    return result;
}

// Advances s.data and decrements s.count
template <typename T>
void
advance(Slice<T> *s, usize count)
{
    assert(count <= s->count);
    s->data += count;
    s->count -= count;
}

// Creates a slice from a C array.
template <typename T, usize N>
constexpr Slice<T>
as_slice(T (&array)[N])
{
    return Slice{N, array};
}

// Removes the item at index `i` by copying the last element in the slice to
// the `i` position. Use `remove_ordered` if the order of elements in the slice
// matters.
template <typename T>
void
remove(Slice<T> *slice, usize i)
{
    --slice->count;
    if (i != slice->count) {
        slice->data[i] = slice->data[slice->count];
    }
}

// Removes an item at index `i` in the slice mantaining the order of elements.
template <typename T>
void
remove_ordered(Slice<T> *slice, usize i)
{
    --slice->count;
    if (i != slice->count) {
        usize count = slice->count - i;
        memcpy(&slice->data[i], &slice->data[i + 1], count * sizeof(T));
    }
}

// Finds an item in the slice. Returns NULL if the item is not found.
template <typename T>
T *
find(Slice<T> slice, T item)
{
    for (auto &it : slice) {
        if (it == item) {
            return &it;
        }
    }
    return NULL;
}

// Finds an item starting from the end of the slice. Returns NULL if no item is
// found.
template <typename T>
T *
rfind(Slice<T> slice, T item)
{
    if (slice.count == 0) return NULL;
    for (auto it = end(slice) - 1; it >= slice.data; --it) {
        if (*it == item) {
            return it;
        }
    }
    return NULL;
}

#endif // CORE_SLICE_H_
