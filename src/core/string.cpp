#include "core/string.h"

// For UTF-8 implementation refer to https://en.wikipedia.org/wiki/UTF-8
static const u8 utf8_first_byte_lengths[32] = {
    // 1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0,
};

char *
make_cstring(Slice<u8> str, Allocator *allocator)
{
    char *cstr = (char *)mem::alloc(str.count + 1, allocator);
    memcpy(cstr, str.data, str.count);
    cstr[str.count] = 0;
    return cstr;
}

Slice<u8>
string_vsprintf(const char *fmt, va_list args, Allocator *allocator)
{
    va_list copy;
    usize len; // number of bytes not including NUL
    Slice<u8> str;

    va_copy(copy, args);
    len = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    str.data = (u8 *)mem::alloc(len + 1, allocator);
    str.count = vsnprintf((char *)str.data, len + 1, fmt, args);
    va_end(args);
    return str;
}

Slice<u8>
string_sprintf(Allocator *allocator, const char *fmt, ...)
{
    va_list args;
    Slice<u8> str;

    va_start(args, fmt);
    str = string_vsprintf(fmt, args, allocator);
    va_end(args);
    return str;
}

Slice<u8>
string_sprintf(const char *fmt, ...)
{
    va_list args;
    Slice<u8> str;

    va_start(args, fmt);
    str = string_vsprintf(fmt, args);
    va_end(args);
    return str;
}

usize
string_encode_rune(Slice<u8> str, u32 rune)
{
    if (rune < 0x80 && 0 < str.count) {
        str.data[0] = (u8)rune;
        return 1;
    }
    if (rune < 0x800 && 1 < str.count) {
        str.data[0] = (u8)(0xC0 | (rune >> 6));
        str.data[1] = (u8)(0x80 | (rune & 0x3F));
        return 2;
    }
    if (rune < 0x10000 && 2 < str.count) {
        if (rune >= 0xD800 && rune <= 0xDFFF) {
            // Surrogate half
            return 0;
        }
        str.data[0] = (u8)(0xE0 | (rune >> 12));
        str.data[1] = (u8)(0x80 | ((rune >> 6) & 0x3F));
        str.data[2] = (u8)(0x80 | (rune & 0x3F));
        return 3;
    }
    if (rune < 0x110000 && 3 < str.count) {
        str.data[0] = (u8)(0xF0 | (rune >> 18));
        str.data[1] = (u8)(0x80 | ((rune >> 12) & 0x3F));
        str.data[2] = (u8)(0x80 | ((rune >> 6) & 0x3F));
        str.data[3] = (u8)(0x80 | (rune & 0x3F));
        return 4;
    }

    // Invalid code point or the string does not have enough space.
    return 0;
}

usize
string_decode_rune(Slice<u8> str, u32 *rune)
{
#define utf8_decode_cont(val, cont)                     \
    if ((cont & 0xC0) != 0x80)                          \
        goto err; /* Expected continuation */           \
    val = (val << 6) | (cont & 0x3F)

    if (str.count == 0) {
        *rune = 0xFFFD;
        return 0;
    }
    u32 len = utf8_first_byte_lengths[str.data[0] >> 3];
    if (len == 1) {
        // ASCII fast-path
        *rune = str.data[0];
        return 1;
    }
    if (str.count < len) {
        goto err; // Missing continuation bytes
    }
    switch (len) {
    case 2: {
        u32 value = str.data[0] & 0x1F;
        utf8_decode_cont(value, str.data[1]);
        if (value < 0x80) goto err; // Overlong sequence
        *rune = value;
        return 2;
    }
    case 3: {
        u32 value = str.data[0] & 0x0F;
        utf8_decode_cont(value, str.data[1]);
        utf8_decode_cont(value, str.data[2]);
        if (value < 0x800) goto err; // Overlong sequence
        // Surrogate half
        if (value >= 0xD800 && value <= 0xDFFF) goto err;
        *rune = value;
        return 3;
    }
    case 4: {
        u32 value = str.data[0] & 0x07;
        utf8_decode_cont(value, str.data[1]);
        utf8_decode_cont(value, str.data[2]);
        utf8_decode_cont(value, str.data[3]);
        if (value < 0x10000) goto err; // Overlong sequence
        if (value > 0x10FFFF) goto err; // Outside valid range
        *rune = value;
        return 4;
    }
    default:
        // Invalid leading byte
        goto err;
    }
err:
    *rune = 0xFFFD;
    return 1;
#undef utf8_decode_cont
}

Slice<u8>
string_next_token(Slice<u8> *str, u8 ch)
{
    while (str->count > 0 && is_space(*str->data)) {
        advance(str, 1);
    }
    auto result = Slice{0, str->data};
    for (; str->count > 0; advance(str, 1), ++result.count) {
        if (*str->data == ch) {
            advance(str, 1);
            break;
        }
    }
    return result;
}

Slice<u8>
string_next_token(Slice<u8> *str)
{
    while (str->count > 0 && is_space(*str->data)) {
        advance(str, 1);
    }
    auto result = Slice{0, str->data};
    for (; str->count > 0; advance(str, 1), ++result.count) {
        if (is_space(*str->data)) {
            advance(str, 1);
            break;
        }
    }
    return result;
}

Slice<u8>
string_next_line(Slice<u8> *str)
{
    auto result = Slice{0, str->data};
    for (; str->count > 0; advance(str, 1), ++result.count) {
        if (*str->data == '\r') {
            advance(str, 1);
            if (str->count > 0 && *str->data == '\n') {
                advance(str, 1);
            }
            break;
        }
        if (*str->data == '\n') {
            advance(str, 1);
            break;
        }
    }
    return result;
}

Slice<u8>
string_trim(Slice<u8> str)
{
    while (str.count > 0 && is_space(*str.data)) {
        advance(&str, 1);
    }
    while (str.count > 0 && is_space(str.data[str.count - 1])) {
        --str.count;
    }
    return str;
}

Slice<u8>
string_ltrim(Slice<u8> str)
{
    while (str.count > 0 && is_space(*str.data)) {
        advance(&str, 1);
    }
    return str;
}

Slice<u8>
string_rtrim(Slice<u8> str)
{
    while (str.count > 0 && is_space(str.data[str.count - 1])) {
        --str.count;
    }
    return str;
}

bool
string_equal_ignore_case(Slice<u8> a, Slice<u8> b)
{
    if (a.count != b.count) {
        return false;
    }
    for (usize i = 0; i < a.count; ++i) {
        if (to_lower(a.data[i]) != to_lower(b.data[i])) {
            return false;
        }
    }
    return true;
}

bool
string_parse_signed(Slice<u8> str, i64 int_min, i64 int_max, i64 base, i64 *result)
{
    u64 acc = 0;
    bool neg = false;
    u64 ovflimit, ovfmin, ovfmax;

    while (str.count && is_space(str.data[0])) {
        advance(&str, 1);
    }
    if (str.count == 0) goto err;
    if (str.data[0] == '-') {
        neg = true;
        advance(&str, 1);
        if (str.count == 0) goto err;
    }

    if (str.data[0] == '0') {
        if ((base == 0 || base == 16)
                && str.count > 1
                && (str[1] == 'x' || str[1] == 'X')) {
            base = 16;
            advance(&str, 2);
        } else if (base == 0 || base == 8) {
            base = 8;
            advance(&str, 1);
        }
    }
    if (base == 0) base = 10;

    ovflimit = neg ? int_min : int_max;
    ovfmin = ovflimit % base;
    ovfmax = ovflimit / base;

    for (; str.count; ++str.data, --str.count) {
        i8 c = *str.data;
        if (is_digit(c)) {
            c -= '0';
        } else if (is_alpha(c)) {
            c -= is_upper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            goto err;
        }
        if (c >= base) goto err;
        if (acc > ovfmax || (acc == ovfmax && acc > ovfmin)) goto err;

        acc *= base;
        acc += c;
    }

    *result = (i64)acc;
    if (neg) *result = -(*result);
    return true;
err:
    *result = 0;
    return false;
}

bool
string_parse_unsigned(Slice<u8> str, u64 int_max, u64 base, u64 *result)
{
    u64 acc = 0;
    u64 ovfmin, ovfmax;

    while (str.count && is_space(str.data[0])) {
        advance(&str, 1);
    }
    if (str.count == 0) goto err;

    if (str.data[0] == '0') {
        if ((base == 0 || base == 16)
                && str.count > 1
                && (str.data[1] == 'x' || str.data[1] == 'X')) {
            base = 16;
            advance(&str, 2);
        } else if (base == 0 || base == 8) {
            base = 8;
            advance(&str, 1);
        }
    }
    if (base == 0) base = 10;

    ovfmin = int_max % base;
    ovfmax = int_max / base;

    for (; str.count; ++str.data, --str.count) {
        u8 c = *str.data;
        if (is_digit(c)) {
            c -= '0';
        } else if (is_alpha(c)) {
            c -= is_upper(c) ? 'A' - 10 : 'a' - 10;
        } else {
            goto err;
        }
        if (c >= base) goto err;
        if (acc > ovfmax || (acc == ovfmax && acc > ovfmin)) goto err;

        acc *= base;
        acc += c;
    }

    *result = acc;
    return true;
err:
    *result = 0;
    return false;
}

template <>
bool
string_parse_float<f64>(Slice<u8> str, f64 *result)
{
    bool parsed_dot = false;
    bool parsed_e = false;
    bool neg = false;
    f64 num = 0.0;
    i64 e = 0;
    usize i = 0;

    while (str.count && is_space(str.data[0])) {
        advance(&str, 1);
    }
    if (str.count == 0) goto err;

    if (str.data[0] == '-') {
        advance(&str, 1);
        neg = true;
    }
    if (str.count == 0) goto err;

    for (; i < str.count; ++i) {
        char c = str.data[i];
        if (is_digit(c)) {
            num = num * 10.0 + (c - '0');
        } else if (!parsed_dot && c == '.') {
            parsed_dot = true;
            continue;
        } else if (c == 'e' || c == 'E') {
            parsed_e = true;
            break;
        } else {
            goto err;
        }
        if (parsed_dot) --e;
    }

    if (parsed_e) {
        i64 e_part;
        if (i + 1 >= str.count) goto err;
        if (!string_parse_int(slice(str, i + 1), &e_part)) goto err;
        e += e_part;
    }

    if (e < 0) {
        for (; e != 0; ++e) num /= 10;
    } else {
        for (; e != 0; --e) num *= 10;
    }
    *result = neg ? -num : num;
    return true;
err:
    *result = 0.0;
    return false;
}

template <>
bool
string_parse_float<f32>(Slice<u8> str, f32 *result)
{
    f64 result64;
    bool parsed = string_parse_float<f64>(str, &result64);
    *result = (f32)result64;
    return parsed;
}
