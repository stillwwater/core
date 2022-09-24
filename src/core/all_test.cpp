#include "core/array.h"
#include "core/basic.h"
#include "core/math.h"
#include "core/memory.h"
#include "core/test.h"
#include "core/table.h"
#include "core/rand.h"
#include "core/slice.h"
#include "core/string.h"

#include "core/all.cpp"

test("Mallocator")
{
    void *ptr = mallocator.alloc(&mallocator, 256, MAX_ALIGN);
    expect(ptr);
    mallocator.free(&mallocator, ptr);

    ptr = mem::alloc(256);
    expect(ptr);
    ptr = mem::realloc(ptr, 1000);
    expect(ptr);
    mem::free(ptr);
}

test("make")
{
    struct Foo { int x, y, z; };
    auto foo = make<Foo>();
    expect(foo);
    mem::free(foo);
}

test("Arena")
{
    auto arena = make_arena(512);
    defer(mem::free(arena));

    expect(arena);
    memset(arena->data, 0xFF, arena->capacity);

    // 400 bytes (+ 16 header (8 align)). allocated: 416
    int *a = (int *)arena_alloc(arena, sizeof(int) * 100, 16);
    expect(a);
    expect((uintptr_t)a % 16 == 0);
    expect(*a == -1);
    expect(((usize *)a)[-1] == sizeof(int) * 100);
    expect(arena->allocated == 416);

    //   4 bytes (+ 16 header (8 align)). allocated: 436
    int *b = (int *)arena_alloc(arena, sizeof(int) * 1, 16);
    expect(b);
    expect((uintptr_t)b % 16 == 0);
    expect(*b == -1);
    expect(((usize *)b)[-1] == sizeof(int) * 1);
    expect(arena->allocated == 436);

    //   8 bytes (+ 12 header (4 align)). allocated: 456
    int *c = (int *)arena_alloc(arena, sizeof(int) * 2, 16);
    expect(c);
    expect((uintptr_t)c % 16 == 0);
    expect(*c == -1);
    expect(((usize *)c)[-1] == sizeof(int) * 2);
    expect(arena->allocated == 456);

    //   4 bytes (+  8 header (0 align)). allocated: 468
    int *d = (int *)arena_alloc(arena, sizeof(int) * 1, 16);
    expect(d);
    expect((uintptr_t)d % 16 == 0);
    expect(*d == -1);
    expect(((usize *)d)[-1] = sizeof(int) * 1);
    expect(arena->allocated == 468);

    //   12 bytes (previously 4). allocated: 476
    int *d_copy = (int *)arena_realloc(arena, d, sizeof(int) * 3, 16);
    expect(d == d_copy);
    expect(d_copy[2] == -1);
    expect(((usize *)d_copy)[-1] == sizeof(int) * 3);
    expect(arena->allocated == 476);

    //   8 bytes (previously 12). allocated: 472
    d_copy = (int *)arena_realloc(arena, d, sizeof(int) * 2, 16);
    expect(d == d_copy);
    expect(d_copy[1] == -1);
    expect(((usize *)d_copy)[-1] == sizeof(int) * 2);
    expect(arena->allocated == 472);

    //   4 bytes (+ 8 header (0 align)). allocated: 484
    int *c_copy = (int *)arena_realloc(arena, c, sizeof(int) * 1, 16);
    expect(c != c_copy);
    expect(arena->allocated == 484);
}

test("Linear Allocator")
{
    auto arena = make_arena(512);
    defer(mem::free(arena));
    expect(arena);

    auto alloc = make_linear_allocator(arena);
    int *a = (int *)mem::alloc(sizeof(int), &alloc);
    expect(alloc.arena->allocated == 20);
    expect(a);

    auto temp = make_linear_allocator(arena);
    int *b = (int *)mem::alloc(sizeof(int), &alloc);
    expect(alloc.arena->allocated == 36);
    expect(b);

    deinit(temp);
    expect(alloc.arena->allocated == 20);
}

test("nextpow2")
{
    expect(nextpow2(1) == 1);
    expect(nextpow2(16) == 16);
    expect(nextpow2(17) == 32);
    expect(nextpow2(2000) == 2048);
}

test("alignptr")
{
    expect(alignptr((void *)0x1, 16) == (void *)0x10);
    expect(alignptr((void *)0x7ffff2, 16) == (void *)0x800000);
}

test("panic")
{
    auto default_panic_proc = core_panic_proc;
    core_panic_proc = [](auto, auto, auto, auto) {};
    panic("test");
    panic("test %d", 2);
    core_panic_proc = default_panic_proc;
}

test("Slice")
{
    int buffer[] = {1, 2, 3};
    auto slice = Slice{3, buffer};
    expect(slice);
    expect(slice.count == 3);
    expect(slice.data == buffer);
    for ([[maybe_unused]] int i : slice) {}

    slice[0] *= 2;
    expect(slice[0] == buffer[0]);
    expect(slice[0] == 2);
}

test("make(N)")
{
    auto nums = make<int>(32);
    defer(mem::free(nums.data));
    expect(nums);
    for (usize i = 0; i < nums.count; ++i) {
        nums[i] = 1 << (int)i;
    }
    auto copy = make_copy(nums);
    defer(mem::free(copy.data));
    expect(copy);
    expect(nums.data != copy.data);
    expect(nums == copy);
}

test("slice")
{
    int arr_a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int arr_b[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto a = as_slice(arr_a);
    auto b = as_slice(arr_b);

    static_assert(is_slice<decltype(a)>());
    static_assert(!is_slice<decltype(arr_a)>());

    expect(a);
    expect(a == b);

    a = slice(a, 1);
    expect(a != b);
    expect(slice(b, 1).count == b.count - 1);
    expect(slice(a, 1) == slice(b, 2));
    expect(slice(a, 1, 2).count == 1);
    expect(slice(a, 1, 2) == slice(b, 2, 3));
    expect(a[0] == 2);
    expect(slice(slice(b, 1), 1)[0] == 3);
}

test("remove")
{
    int arr_before[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int arr_after[] = {1, 2, 3, 4, 9, 6, 7, 8};
    auto before = as_slice(arr_before);
    auto after = as_slice(arr_after);
    remove(&before, 4);
    expect(before == after);
}

test("remove ordered")
{
    int arr_before[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int arr_after[] = {1, 2, 3, 4, 6, 7, 8, 9};
    auto before = as_slice(arr_before);
    auto after = as_slice(arr_after);
    remove_ordered(&before, 4);
    expect(before == after);
}

test("find")
{
    int arr_a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto a = as_slice(arr_a);
    expect(find(a, 9) - a.data == 8);
    expect(rfind(a, 1) - a.data == 0);
}

test("string_sprintf")
{
    auto alloc = make_linear_allocator(make_arena(4096));
    defer(mem::free(alloc.arena));
    expect(string_sprintf(&alloc, "%s:%d", "abc", 123) == "abc:123"_u8);
    expect(string_sprintf(&alloc, "%s", "") == ""_u8);
}

test("string_parse_int")
{
    int x0;
    bool ok0 = string_parse_int("  -10"_u8, &x0);
    expect(ok0);
    expect(x0 == -10);

    int x1;
    bool ok1 = string_parse_int("EF5350"_u8, &x1, 16);
    expect(ok1);
    expect(x1 == 0xEF5350);

    u32 x2;
    bool ok2 = string_parse_int<u32>("0xEF5350"_u8, &x2);
    expect(ok2);
    expect(x2 == 0xEF5350);

    u8 x3;
    bool ok3 = string_parse_int<u8>("10000"_u8, &x3, 2);
    expect(ok3);
    expect(x3 == 16);

    u8 x4;
    bool ok4 = string_parse_int<u8>("012"_u8, &x4, 10);
    expect(ok4);
    expect(x4 == 12);

    u32 x5;
    bool ok5 = string_parse_int<u32>("-10"_u8, &x5);
    expect(!ok5);
    (void)x5;

    u8 x6;
    bool ok6 = string_parse_int<u8>("256"_u8, &x6);
    expect(!ok6);
    (void)x6;

    i32 x7;
    bool ok7 = string_parse_int("10zzz"_u8, &x7);
    expect(!ok7);
    (void)x7;

    i32 x8;
    bool ok8 = string_parse_int("EF5350"_u8, &x8, 10);
    expect(!ok8);
    (void)x8;
}

test("string_parse_float")
{
    f32 x0;
    bool ok0 = string_parse_float("  -1"_u8, &x0);
    expect(ok0);
    expect(x0 == -1.0f);

    f32 x1;
    bool ok1 = string_parse_float(".26"_u8, &x1);
    expect(ok1);
    expect(x1 == 0.26f);

    f64 x2;
    bool ok2 = string_parse_float<f64>("0.0"_u8, &x2);
    expect(ok2);
    expect(x2 == 0.0);

    f64 x3;
    bool ok3 = string_parse_float<f64>("inf"_u8, &x3);
    expect(ok3);
    (void)x3;

    f64 x4;
    bool ok4 = string_parse_float<f64>("-inf"_u8, &x4);
    expect(ok4);
    (void)x4;

    f64 x5;
    bool ok5 = string_parse_float<f64>("nan"_u8, &x5);
    expect(ok5);
    (void)x5;

    f32 x6;
    bool ok6 = string_parse_float("10.0  "_u8, &x6);
    expect(!ok6);
    (void)x6;

    f32 x7;
    bool ok7 = string_parse_float("10zzz"_u8, &x7);
    expect(!ok7);
    (void)x7;
}

test("string_next_token")
{
    auto t1 = "a * b"_u8;
    expect(t1.count == 5);
    expect(string_next_token(&t1, ' ') == "a"_u8);
    expect(string_next_token(&t1, ' ') == "*"_u8);
    expect(t1 == "b"_u8);

    auto t2 = ""_u8;
    expect(string_next_token(&t2, ' ') == ""_u8);
    expect(t2 == ""_u8);

    auto t3 = " a b"_u8;
    expect(string_next_token(&t3, ' ') == "a"_u8);
    expect(t3 == "b"_u8);

    auto t4 = "abcd"_u8;
    expect(string_next_token(&t4, 'd') == "abc"_u8);
    expect(t4 == ""_u8);

    auto t5 = "a *\tb\n"_u8;
    expect(string_next_token(&t5) == "a"_u8);
    expect(string_next_token(&t5) == "*"_u8);
    expect(string_next_token(&t5) == "b"_u8);
    expect(t5 == ""_u8);
    expect(string_next_token(&t5) == ""_u8);
    expect(string_next_token(&t5) == ""_u8);
}

test("string_next_line")
{
    auto t1 = "abc\ndef\n"_u8;
    expect(string_next_line(&t1) == "abc"_u8);
    expect(string_next_line(&t1) == "def"_u8);
    expect(t1 == ""_u8);

    auto t2 = "abc\ndef"_u8;
    expect(string_next_line(&t2) == "abc"_u8);
    expect(string_next_line(&t2) == "def"_u8);
    expect(t2 == ""_u8);

    auto t3 = "abc\r\ndef"_u8;
    expect(string_next_line(&t3) == "abc"_u8);
    expect(string_next_line(&t3) == "def"_u8);
    expect(t3 == ""_u8);

    auto t4 = "abc\rdef"_u8;
    expect(string_next_line(&t4) == "abc"_u8);
    expect(string_next_line(&t4) == "def"_u8);
    expect(t4 == ""_u8);

    auto t5 = "abc\ndef\n\n"_u8;
    expect(string_next_line(&t5) == "abc"_u8);
    expect(string_next_line(&t5) == "def"_u8);
    expect(t5 == "\n"_u8);
}

test("string_trim")
{
    expect(string_ltrim("\n   a "_u8) == "a "_u8);
    expect(string_ltrim(""_u8) == ""_u8);
    expect(string_ltrim(" "_u8) == ""_u8);
    expect(string_rtrim(" a "_u8) == " a"_u8);
    expect(string_trim(" a "_u8) == "a"_u8);
}

test("string_equal_ignore_case")
{
    expect(string_equal_ignore_case("Hello"_u8, "hello"_u8));
    expect(string_equal_ignore_case("abc"_u8, "abc"_u8));
    expect(string_equal_ignore_case(""_u8, ""_u8));
    expect(!string_equal_ignore_case("abc"_u8, "axb"_u8));
}

test("utf8 encode")
{
    u8 array[4];
    auto str = as_slice(array);

    expect(string_encode_rune(str, '$') == 1);
    expect(str[0] == '$');

    expect(string_encode_rune(str, 0xA3) == 2);
    expect(str[0] == 0xC2);
    expect(str[1] == 0xA3);

    expect(string_encode_rune(str, 0xD55C) == 3);
    expect(str[0] == 0xED);
    expect(str[1] == 0x95);
    expect(str[2] == 0x9C);

    expect(string_encode_rune(str, 0x10348) == 4);
    expect(str[0] == 0xF0);
    expect(str[1] == 0x90);
    expect(str[2] == 0x8D);
    expect(str[3] == 0x88);

    expect(string_encode_rune(str, 0xD800) == 0);
    expect(string_encode_rune(str, 0xDFFF) == 0);
    expect(string_encode_rune(str, 0x110000) == 0);
}

test("utf8 decode")
{
    u32 rune;

    // Valid encodings
    expect((string_decode_rune("\0"_u8, &rune) == 1 && rune == 0x0));
    expect((string_decode_rune("A"_u8, &rune) == 1 && rune == 'A'));
    expect((string_decode_rune("\x7F"_u8, &rune) == 1 && rune == 0x7F));
    expect((string_decode_rune("\xC2\x80"_u8, &rune) == 2 && rune == 0x80));
    expect((string_decode_rune("\xDF\xBF"_u8, &rune) == 2 && rune == 0x7FF));
    expect((string_decode_rune("\xE0\xA0\x80"_u8, &rune) == 3 && rune == 0x800));
    expect((string_decode_rune("\xEF\xBF\xBF"_u8, &rune) == 3 && rune == 0xFFFF));
    expect((string_decode_rune("\xEF\xBF\xBD"_u8, &rune) == 3 && rune == 0xFFFD));
    expect((string_decode_rune("\xF0\x90\x80\x80"_u8, &rune) == 4 && rune == 0x10000));
    expect((string_decode_rune("\xF4\x8F\xBF\xBF"_u8, &rune) == 4 && rune == 0x10FFFF));

    // Invalid first byte
    expect((string_decode_rune("\xFF"_u8, &rune) == 1 && rune == 0xFFFD));

    // Unexpected continuation byte
    expect((string_decode_rune("\x80"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xBF"_u8, &rune) == 1 && rune == 0xFFFD));

    // Expected 1 continuation byte
    expect((string_decode_rune("\xC2"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xC2\0"_u8, &rune) == 1 && rune == 0xFFFD));

    // Expected 2 continuation bytes
    expect((string_decode_rune("\xED"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xED\x95"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xED\0\0"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xED\x95\0"_u8, &rune) == 1 && rune == 0xFFFD));

    // Expected 3 continuation bytes
    expect((string_decode_rune("\xF0"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xF0\x90"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xF0\x90\x8D"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xF0\0\0\0"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xF0\x90\0\0"_u8, &rune) == 1 && rune == 0xFFFD));
    expect((string_decode_rune("\xF0\x90\x8D\0"_u8, &rune) == 1 && rune == 0xFFFD));

    // Overlong sequence
    expect((string_decode_rune("\xF0\x82\x82\xAC"_u8, &rune) == 1 && rune == 0xFFFD));
}

test("Array")
{
    auto array = make_array<int>();
    append(&array, 10);
    append(&array, 20, 30);
    expect(array.count == 3);
    expect(array.capacity == 4);
    expect(array[0] == 10);
    expect(array[1] == 20);
    expect(array[2] == 30);

    int arr[] = {40, 50, 60};
    auto slice = as_slice(arr);
    append_slice(&array, slice);
    expect(array.count == 6);
    expect(array.capacity == 6);
    expect(array[3] == 40);
    expect(array[4] == 50);
    expect(array[5] == 60);

    array.count = 0; // Clear the array
    append(&array, 70);
    expect(array.count == 1);
    expect(array.capacity == 6);
    expect(array[0] == 70);

    array_trim(&array);
    expect(array.count == 1);
    expect(array.capacity == 1);
    expect(array[0] == 70);

    array_resize(&array, 3);
    expect(array.count == 3);
    expect(array.capacity == 3);
    array[2] = 90;
    expect(array[0] == 70);
    expect(array[2] == 90);

    array[1] = 80;
    auto copy = make_array_copy(array);
    expect(array == copy);
    copy[0] = 10;
    expect(array[0] != copy[0]);

    deinit(array);
    deinit(copy);
}

test("rand")
{
    // 24 first values from reference implementation with seed=0xC0FFEE
    // > https://prng.di.unimi.i
    u64 samples[24] = {
        0x120E99A6DDE4A550, 0x8F989EF97733D4B4, 0xF0A28EB2E4FD367B,
        0x50C29BFE8734F5D2, 0xF763EB3E1CBE4E9B, 0x4ECA86E0293E9B6C,
        0x534AFA30DAEECA16, 0xFBCC18B345689622, 0x1794F3F570E31D45,
        0x4F1E8A580D4CDF33, 0x75BB72F5DE33F8A4, 0xCDA3B71566E0B14D,
        0x603901A92988579D, 0xB6649FF62CB3F5E5, 0xD9A49720163B9AB3,
        0xE1E8F07445E5314D, 0x539474E78425BCE2, 0xE695176C83AD2085,
        0x1D2FFAE72F54124A, 0x77B8CF6A28A4F078, 0x43C5697CC7FFAB9E,
        0x33001290A5B93EC9, 0x38D0BC1CFD3797DC, 0x901F901136624A26,
    };

    Xoshiro256 rng;
    rand_init(&rng, 0xC0FFEE);
    for (usize i = 0; i < 24; ++i) {
        expect(rand_next(&rng) == samples[i]);
    }

    for (usize i = 0; i < 64; ++i) {
        f32 f = rand_float(&rng);
        f64 d = rand_float<f64>(&rng);
        expect(f >= 0.0f && f < 1.0f);
        expect(d >= 0.0 && d < 1.0);
    }

    (void)rand_float(&rng, 0.0f, 10.0f);
    (void)rand_int(&rng);
    (void)rand_int<u64>(&rng);
    (void)rand_int<u64>(&rng, 0, 100);
    (void)rand_int<char>(&rng);
}

test("Table")
{
    auto table = make_table<int, int>();
    defer(deinit(table));

    expect(table_find(&table, 0) == NULL);
    expect(table_find(&table, 1) == NULL);

    for (int i = 0; i < 24; ++i) {
        expect(*table_update(&table, i, i * 10) == i * 10);
    }

    expect(table.count == 24);
    for (int i = 0; i < 24; ++i) {
        expect(table_find(&table, i));
        expect(*table_find(&table, i) == i * 10);
    }

    for (int i = 0; i < 24; ++i) {
        expect(*table_update(&table, i, i * 100) == i * 100);
    }

    expect(table.count == 24);
    for (int i = 0; i < 24; ++i) {
        expect(table_find(&table, i));
        expect(*table_find(&table, i) == i * 100);
    }

    for (int i = 0; i < 24; ++i) {
        expect(table_remove(&table, i));
    }
    expect(table.count == 0);

    for (int i = 0; i < 24; ++i) {
        expect(*table_update(&table, i, i * 1000) == i * 1000);
    }

    expect(table.count == 24);
    for (int i = 0; i < 24; ++i) {
        expect(table_find(&table, i));
        expect(*table_find(&table, i) == i * 1000);
    }

    for (auto &[key, val] : table) {
        expect(val == key * 1000);
    }
}

test("Table string")
{
    auto table = make_table<Slice<u8>, int>();
    defer(deinit(table));
    expect(*table_update(&table, "zero"_u8, 0) == 0);
    expect(table_find(&table, "zero"_u8));
    expect(*table_find(&table, "zero"_u8) == 0);
    expect(table.count == 1);
    expect((table.capacity == Table<Slice<u8>, int>::MIN_CAPACITY));

    expect(!table_find(&table, "one"_u8));
    expect(*table_update(&table, "zero"_u8, 1) == 1);
    expect(table_find(&table, "zero"_u8));
    expect(*table_find(&table, "zero"_u8) == 1);

    expect(table_remove(&table, "zero"_u8));
    expect(table.count == 0);
    table["zero"_u8] = 10;
    expect(table_find(&table, "zero"_u8));
    expect(table["zero"_u8] == 10);
    expect(table.count == 1);
}

test("Table badhash")
{
    // Hash function with 100% collisions
    auto badhash = [](int *key) -> u64 {
        (void)key;
        return ~0ull;
    };

    auto table = make_table<int, int>(badhash);
    defer(deinit(table));

    for (int i = 0; i < 24; ++i) {
        expect(*table_update(&table, i, i * 10) == i * 10);
    }

    expect(table.count == 24);
    for (int i = 0; i < 24; ++i) {
        expect(table_find(&table, i));
        expect(*table_find(&table, i) == i * 10);
    }
}

test("Matrix inverse")
{
    auto in3  = mat3({1, 1, 0}, {0, 2, 2}, {0, 0, 3});
    auto out3 = mat3({1, -1/2.0f, 1/3.0f}, {0, 1/2.0f, -1/3.0f}, {0, 0, 1/3.0f});
    expect(math::equal(math::inverse(in3), out3));

    auto in4  = mat4({1, 1, 0, 0}, {0, 2, 2, 0}, {0, 0, 1, 0}, {0, 0, 2, 1});
    auto out4 = mat4({1, -1/2.0f, 1, 0}, {0, 1/2.0f, -1, 0}, {0, 0, 1, 0}, {0, 0, -2, 1});
    expect(math::equal(math::inverse(in4), out4));
    expect(math::equal(math::affine_inverse(in4), out4));
}

test("math template")
{
    // Typecheck math.h
    (void)(vec2(0) + vec2(0));
    (void)(vec2(0) - vec2(0));
    (void)(vec2(0) * vec2(0));
    (void)(vec2(0) / vec2(1));
    (void)(vec2(0) * 1.0f);
    (void)(vec2(0) / 1.0f);
    (void)(-vec2(0));
    (void)(vec2(0)[0]);
    (void)(vec2(0) == vec2(0));
    (void)(vec2(0) != vec2(0));
    (void)(point2(0) + point2(0));
    (void)(point2(0) - point2(0));
    (void)(point2(0) * point2(0));
    (void)(point2(0) / point2(1));
    (void)(point2(0) * 1);
    (void)(point2(0) / 1);
    (void)(-point2(0));
    (void)(point2(0)[0]);
    (void)(point2(0) == point2(0));
    (void)(point2(0) != point2(0));
    (void)(vec3(0) + vec3(0));
    (void)(vec3(0) - vec3(0));
    (void)(vec3(0) * vec3(0));
    (void)(vec3(0) / vec3(1));
    (void)(vec3(0) * 1.0f);
    (void)(vec3(0) / 1.0f);
    (void)(-vec3(0));
    (void)(+vec3(0));
    (void)(vec3(0)[0]);
    (void)(vec3(0) == vec3(0));
    (void)(vec3(0) != vec3(0));
    (void)(vec4(0) + vec4(0));
    (void)(vec4(0) - vec4(0));
    (void)(vec4(0) * vec4(0));
    (void)(vec4(0) / vec4(1));
    (void)(vec4(0) * 1.0f);
    (void)(vec4(0) / 1.0f);
    (void)(-vec4(0));
    (void)(+vec4(0));
    (void)(vec4(0)[0]);
    (void)(vec4(0) == vec4(0));
    (void)(vec4(0) != vec4(0));

    (void)(mat3(1) + mat3(1));
    (void)(mat3(1) - mat3(1));
    (void)(mat3(1) * mat3(1));
    (void)(mat3(1) * vec3(1));
    (void)(mat3(1) == mat3(1));
    (void)(mat3(1) != mat3(1));
    expect(math::transpose(mat3(1)) == mat3(1));
    expect(math::inverse(mat3(1)) == mat3(1));
    (void)(mat4(1) + mat4(1));
    (void)(mat4(1) - mat4(1));
    (void)(mat4(1) * mat4(1));
    (void)(mat4(1) * vec4(1));
    (void)(mat4(1) == mat4(1));
    (void)(mat4(1) != mat4(1));
    expect(math::transpose(mat4(1)) == mat4(1));
    expect(math::inverse(mat4(1)) == mat4(1));
    expect(math::affine_inverse(mat4(1)) == mat4(1));

    (void)(math::contains(math::rect(0, 0, 1, 1), math::rect(0, 0, 1, 1)));
    (void)(math::intersects(math::rect(0, 0, 1, 1), math::rect(0, 0, 1, 1)));
    (void)(math::contains(math::quad(0, 0, 1, 1), vec2(0.5, 0.5)));

    (void)(math::equal(0.0f, 0.0f));
    (void)(math::min(0.0f, 0.0f));
    (void)(math::max(0.0f, 0.0f));
    (void)(math::sign(1.0f));
    (void)(math::clamp(1.5f, 0.0f, 1.0f));
    (void)(math::saturate(1.5f));
    (void)(math::lerp(0.0f, 1.0f, 0.5f));
    (void)(math::lerp(vec3(0), vec3(1), 0.5f));
    (void)(math::slerp(vec3(0), vec3(1), 0.5f));
    (void)(math::dot(vec2(0), vec2(1)));
    (void)(math::dot(vec3(0), vec3(1)));
    (void)(math::dot(vec4(0), vec4(1)));
    (void)(math::cross(vec3(0), vec3(1)));
    (void)(math::length(vec3(1)));
    (void)(math::length2(vec3(1)));
    (void)(math::normalize(vec3(1)));
    (void)(math::floor(vec3(1)));
    (void)(math::floortoi(vec3(1)));
    (void)(math::trunc(vec3(1)));
    (void)(math::round(vec3(1)));
    (void)(math::ceil(vec3(1)));
    (void)(math::radians(vec3(1)));
    (void)(math::degrees(vec3(1)));
    (void)(math::equal(vec3(0), vec3(0)));
    (void)(math::shuffle<0, 1>(vec4(0)));
    (void)(math::shuffle<0, 1, 2>(vec4(0)));
    (void)(math::shuffle<0, 1, 2, 3>(vec4(0)));

    (void)(math::perspective(math::radians(70.0f), 1920.0f/1080.0f, 0.1f, 100.0f));
    (void)(math::lookat(vec3(0), vec3(0, 1, 0), vec3(0, 0, 1)));
    (void)(math::translation(vec3(0)));
    (void)(math::rotation(math::quat(vec3(0))));
    (void)(math::scale(vec3(1)));
    (void)(math::quat(vec3(0, 0, 1), math::PI_2));
    (void)(math::quat_axis(math::quat()));
    (void)(math::quat_angle(math::quat()));
    (void)(math::euler(math::quat()));
    (void)(math::conjugate(math::quat()));
    (void)(math::qmul(math::quat(), math::quat()));
}
