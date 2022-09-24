#ifndef CORE_ARRAY_H_
#define CORE_ARRAY_H_

#include <string.h>

#include "core/basic.h"
#include "core/memory.h"
#include "core/slice.h"

// Contiguos dynamic array:
// * Arrays own memory which is freed with deinit(Array).
// * Does not support RAII types, destructors will not be called.
template <typename T>
struct Array : Slice<T> {
    usize capacity;
    Allocator *allocator;
};

// Allocates a new array.
template <typename T>
Array<T>
make_array(ALLOCATOR_PARAM(allocator))
{
    Array<T> array;
    array.count = 0;
    array.data = NULL;
    array.capacity = 0;
    array.allocator = allocator;
    return array;
}

// Allocates a new array and copies items from the source array.
template <typename T>
Array<T>
make_array_copy(Array<T> array)
{
    Array<T> copy;
    copy.count = array.count;
    copy.data = (T *)mem::alloc(
            array.count * sizeof(T), min_alignof<T>(), array.allocator);
    copy.capacity = array.count;
    copy.allocator = array.allocator;
    memcpy(copy.data, array.data, array.count * sizeof(T));
    return copy;
}

template <typename T>
void
deinit(Array<T> array)
{
    // ok if .data is NULL.
    mem::free(array.data, array.allocator);
}

// Resizes the buffer to an exact count making .count equal to .capacity. If
// the given count is smaller than the current count items at the end of the
// array will be dropped. If it is larger, new items will be uninitialized.
// If count is 0, the array data is deallocated.
template <typename T>
void
array_resize(Array<T> *array, usize count)
{
    if (count == array->capacity) {
        array->count = count;
        return;
    }
    if (count == 0) {
        mem::free(array->data, array->allocator);
        array->count = 0;
        array->data = NULL;
        array->capacity = 0;
        return;
    }
    array->count = count;
    array->data = (T *)mem::realloc(
            array->data, count * sizeof(T),
            min_alignof<T>(), array->allocator);
    array->capacity = count;
}

// Ensures the array has at least a given capacity. The underlying buffer is
// resized if needed.
template <typename T>
void
array_reserve(Array<T> *array, usize capacity)
{
    if (capacity > array->capacity) {
        array->capacity = capacity;
        array->data = (T *)mem::realloc(
                array->data, capacity * sizeof(T),
                min_alignof<T>(), array->allocator);
    }
}

// Shrinks the array such that .count is equal to .capacity, trimming unused
// capacity.
template <typename T>
void
array_trim(Array<T> *array)
{
    array_resize(array, array->count);
}

// Appends an item to the end of the array and increases count. The array is
// resized if needed.
template <typename T>
void
append(Array<T> *array, T value)
{
    if (array->count == array->capacity) {
        array->capacity = array->capacity != 0 ? array->capacity << 1 : 1;
        array->data = (T *)mem::realloc(
                array->data, array->capacity * sizeof(T),
                min_alignof<T>(), array->allocator);
    }
    array->data[array->count] = value;
    ++array->count;
}

// Appends items to the end of the array and increases count. The array is
// resized if needed.
template <typename T, typename... Ts>
void
append(Array<T> *array, T value, Ts... values)
{
    append(array, value);
    (append(array, values), ...);
}

// Appends items from a slice to the end of the array and increases count. The
// array is resized if needed.
template <typename T>
void
append_slice(Array<T> *array, Slice<T> values)
{
    array_reserve(array, array->count + values.count);
    for (auto it : values) {
        array->data[array->count] = it;
        ++array->count;
    }
}

#endif // CORE_ARRAY_H_
