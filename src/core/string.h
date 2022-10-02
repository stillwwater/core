#ifndef CORE_STRING_H_
#define CORE_STRING_H_

#include <limits>
#include <string.h>

#include "core/basic.h"
#include "core/macro.h"
#include "core/memory.h"
#include "core/slice.h"

#define strfmt(str) (int)str.count, (const char *)str.data

inline Slice<u8>
operator""_u8(const char *str, usize count)
{
    return Slice<u8>{count, (u8 *)str};
}

// Allocates a copy of the string as a NUL terminated C string.
char *make_cstring(Slice<u8> str, ALLOCATOR_PARAM(allocator));

// Returns a new allocated string.
Slice<u8> string_vsprintf(const char *fmt, va_list args, ALLOCATOR_PARAM(allocator));

// Returns a new allocated string.
Slice<u8> string_sprintf(Allocator *allocator, const char *fmt, ...) M_PRINTF_FMT(2, 3);

// Returns a new allocated string. The default allocator is used to create the
// string.
Slice<u8> string_sprintf(const char *fmt, ...) M_PRINTF_FMT(1, 2);

// Encodes `rune` as a UTF-8 byte sequence and writes it to the input string.
// This function returns the number of bytes written to the input string. If
// the input string is not long enough for the bytes to be written, no bytes
// are written and 0 is returned.
usize string_encode_rune(Slice<u8> str, u32 rune);

// `string_decode_rune` decodes a single rune from a UTF-8 byte sequence. It
// returns a pair of `{rune, length}` where the length is the number of bytes
// used to decode the rune. This function will validate that the rune is valid
// UTF-8, and return the replacement character (`U+FFFD`) in case of errors. If
// the source string is empty, the function returns `{0xFFFD, 0}`. If the rune
// being decoded is not valid UTF-8, this function returns `{0xFFFD, 1}` to
// allow the caller to skip over 1 byte and continue decoding the string. In
// both cases this pair can be used to detect an error as the resulting pairs
// are impossible to occur in a valid UTF-8 string.
usize string_decode_rune(Slice<u8> str, u32 *rune);

// Advances the source string and parses a token until `ch` is found. If `ch`
// is not found in the source string, the result is the entire source string.
// `ch` is consumed such that neither the source or resulting string contain
// the first instance of `ch`.
Slice<u8> string_next_token(Slice<u8> *str, u8 ch);

// Advances the source string and parses a token until a whitespace character
// is found. If the string does not contain any whitespace, the result is the
// entire source string. All trailing whitespace is consumed.
Slice<u8> string_next_token(Slice<u8> *str);

// Advances the source string and parses a line ending in \n, \r, or \r\n.
Slice<u8> string_next_line(Slice<u8> *str);

// Returns a string with leading and trailing whitespace removed.
Slice<u8> string_trim(Slice<u8> str);

// Returns a string with leading whitespace removed.
Slice<u8> string_ltrim(Slice<u8> str);

// Returns a string with trailing whitespace removed.
Slice<u8> string_rtrim(Slice<u8> str);

// Compares two strings, ignoring casing (ASCII only).
bool string_equal_ignore_case(Slice<u8> a, Slice<u8> b);

inline bool
is_space(u8 ch)
{
    return ch == ' ' || ch == '\t' || ch == '\v'
        || ch == '\n' || ch == '\r' || ch == '\f';
}

inline bool
is_upper(u8 ch)
{
    return ch >= 'A' && ch <= 'Z';
}

inline bool
is_lower(u8 ch)
{
    return ch >= 'a' && ch <= 'z';
}

inline bool
is_alpha(u8 ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

inline bool
is_digit(u8 ch)
{
    return ch >= '0' && ch <= '9';
}

inline u8
to_lower(u8 ch)
{
    return (ch >= 65 && ch <= 90) ? ch + 32 : ch;
}

inline u8
to_upper(u8 ch)
{
    return (ch >= 97 && ch <= 122) ? ch - 32 : ch;
}

// Converts a string to a floating point number.
// * By default `f32` is assumed.
// * Leading whitespace is ignored, all other invaid leading or trailing
//   characters will result in an error.
template <typename T = f32>
bool string_parse_float(Slice<u8> str, T *result);

// Converts a string to a signed integer.
bool string_parse_signed(Slice<u8> str, i64 int_min, i64 int_max, i64 base, i64 *result);

// Converts a string to an unsigned integer.
bool string_parse_unsigned(Slice<u8> str, u64 int_max, u64 base, u64 *result);

// Converts a string to an integer.
// * By default `i32` is assumed.
// * If `base` is 0 the int base will be determined by the string format: if
//   the string begins with a `0x` it is assumed to be in base 16, if the
//   string begins with a `0` then base 8 is assumed. Otherwise the default
//   is base 10.
// * If the string begins with a `-` sign, the integer will be negative if it
//   is signed or an error is returned if the integer parameter is unsigned.
// * If the integer parsed would cause an overflow then
//   `numeric_limits<T>::min/max` and an error is returned.
// * Leading whitespace is ignored, all other invalid leading or trailing
//   characters will result in an error.
template <typename T = i32>
bool
string_parse_int(Slice<u8> str, T *result, u64 base = 0)
{
    constexpr auto int_min = std::numeric_limits<T>::min();
    constexpr auto int_max = std::numeric_limits<T>::max();

    if constexpr (int_min < 0) {
        i64 result64;
        bool parsed = string_parse_signed(str, int_min, int_max, base, &result64);
        *result = static_cast<T>(result64);
        return parsed;
    } else {
        u64 result64;
        bool parsed = string_parse_unsigned(str, int_max, base, &result64);
        *result = static_cast<T>(result64);
        return parsed;
    }
}

#endif // CORE_STRING_H_
