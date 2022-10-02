#ifndef CORE_MATH_H_
#define CORE_MATH_H_

#undef min
#undef max

#include <math.h>

#include <limits>
#include <type_traits>

// Conventions:
//
// Coordinate System   Right handed, +Z-up, +Y forward
//
// Rotation Unit       Radians
// Rotation Axis       X: roll, Y: pitch, Z: yaw (positive axis: counter-clockwise)
// Rotation Order      yaw->pitch->roll (ZYX)
//
// Quaternion layout   (x, y, z, w)  where w: real, xyz: imaginary
// Matrix layout       Column-major, index access is reversed since arrays are row-major in C
//                     Matrix[0] returns the first column,
//                     Matrix[0][1] Returns the second element of the first column
// Color layout        (r, g, b, a)
namespace math {

constexpr float e       = float(2.71828182845904523536);
constexpr float PI      = float(3.14159265358979323846);
constexpr float PI_2    = float(1.57079632679489661923);
constexpr float PI_4    = float(0.78539816339744830961);
constexpr float TAU     = float(6.28318530717958647692);
constexpr float SQRT2   = float(1.41421356237309504880);

template <typename T, size_t N>
struct Vector;

template <typename T>
struct Vector<T, 2> {
    enum { SIZE = 2 };

    union {
        struct { T x, y; };
        struct { T u, v; };
    };

    Vector() = default;
    constexpr Vector(T x_, T y_)
        : x(x_)
        , y(y_) {}

    T operator[](size_t i) const { return ((const T *)this)[i]; }
    T &operator[](size_t i) { return ((T *)this)[i]; }

    Vector operator+() const { return *this; };
    Vector operator-() const { return Vector(-x, -y); };
};

template <typename T>
struct Vector<T, 3> {
    enum { SIZE = 3 };

    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        Vector<T, 2> xy;
    };

    Vector() = default;
    constexpr Vector(T x_, T y_, T z_)
        : x(x_)
        , y(y_)
        , z(z_) {}

    T operator[](size_t i) const { return ((const T *)this)[i]; }
    T &operator[](size_t i) { return ((T *)this)[i]; }

    Vector operator+() const { return *this; };
    Vector operator-() const { return Vector(-x, -y, -z); };
};

template <typename T>
struct Vector<T, 4> {
    enum { SIZE = 4 };

    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        struct { Vector<T, 2> xy, zw; };
        Vector<T, 3> xyz;
        Vector<T, 3> rgb;
    };

    Vector() = default;
    constexpr Vector(T x_, T y_, T z_, T w_)
        : x(x_)
        , y(y_)
        , z(z_)
        , w(w_) {}

    T operator[](size_t i) const { return ((const T *)this)[i]; }
    T &operator[](size_t i) { return ((T *)this)[i]; }

    Vector operator+() const { return *this; }
    Vector operator-() const { return Vector(-x, -y, -z, -w); }
};

#define MATH_VEC2_OP(op)                                                      \
    template <typename T>                                                     \
    inline Vector<T, 2>                                                       \
    operator op(Vector<T, 2> a, Vector<T, 2> b)                               \
    {                                                                         \
        return {a.x op b.x, a.y op b.y};                                      \
    }                                                                         \
                                                                              \
    template <typename T, typename S,                                         \
            std::enable_if_t<std::is_scalar_v<S>, bool> = true>               \
    inline Vector<T, 2>                                                       \
    operator op(Vector<T, 2> a, S scalar)                                     \
    {                                                                         \
        return {a.x op scalar, a.y op scalar};                                \
    }                                                                         \
                                                                              \
    template <typename T, typename S,                                         \
            std::enable_if_t<std::is_scalar_v<S>, bool> = true>               \
    inline Vector<T, 2>                                                       \
    operator op(S scalar, Vector<T, 2> a)                                     \
    {                                                                         \
        return {scalar op a.x, scalar op a.y};                                \
    }

#define MATH_VEC3_OP(op)                                                      \
    template <typename T>                                                     \
    inline Vector<T, 3>                                                       \
    operator op(Vector<T, 3> a, Vector<T, 3> b)                               \
    {                                                                         \
        return {a.x op b.x, a.y op b.y, a.z op b.z};                          \
    }                                                                         \
                                                                              \
    template <typename T, typename S,                                         \
            std::enable_if_t<std::is_scalar_v<S>, bool> = true>               \
    inline Vector<T, 3>                                                       \
    operator op(Vector<T, 3> a, S scalar)                                     \
    {                                                                         \
        return {a.x op scalar, a.y op scalar, a.z op scalar};                 \
    }                                                                         \
                                                                              \
    template <typename T, typename S,                                         \
            std::enable_if_t<std::is_scalar_v<S>, bool> = true>               \
    inline Vector<T, 3>                                                       \
    operator op(S scalar, Vector<T, 3> a)                                     \
    {                                                                         \
        return {scalar op a.x, scalar op a.y, scalar op a.z};                 \
    }

#define MATH_VEC4_OP(op)                                                      \
    template <typename T>                                                     \
    inline Vector<T, 4>                                                       \
    operator op(Vector<T, 4> a, Vector<T, 4> b)                               \
    {                                                                         \
        return {a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w};              \
    }                                                                         \
                                                                              \
    template <typename T, typename S,                                         \
            std::enable_if_t<std::is_scalar_v<S>, bool> = true>               \
    inline Vector<T, 4>                                                       \
    operator op(Vector<T, 4> a, S scalar)                                     \
    {                                                                         \
        return {a.x op scalar, a.y op scalar, a.z op scalar, a.w op scalar};  \
    }                                                                         \
                                                                              \
    template <typename T, typename S,                                         \
            std::enable_if_t<std::is_scalar_v<S>, bool> = true>               \
    inline Vector<T, 4>                                                       \
    operator op(S scalar, Vector<T, 4> a)                                     \
    {                                                                         \
        return {scalar op a.x, scalar op a.y, scalar op a.z, scalar op a.w};  \
    }

#define MATH_VEC_OP_ASSIGN(op)                                                \
    template <typename T, size_t N>                                           \
    inline Vector<T, N>                                                       \
    operator op##=(Vector<T, N> &a, Vector<T, N> b)                           \
    {                                                                         \
        a = a op b;                                                           \
        return a;                                                             \
    }                                                                         \
                                                                              \
    template <typename T, size_t N, typename S>                               \
    inline Vector<T, N>                                                       \
    operator op##=(Vector<T, N> &a, S scalar)                                 \
    {                                                                         \
        a = a op scalar;                                                      \
        return a;                                                             \
    }

MATH_VEC2_OP(+)
MATH_VEC2_OP(-)
MATH_VEC2_OP(*)
MATH_VEC2_OP(/)

template <typename T>
bool
operator==(Vector<T, 2> a, Vector<T, 2> b)
{
    return a.x == b.x && a.y == a.y;
}

MATH_VEC3_OP(+)
MATH_VEC3_OP(-)
MATH_VEC3_OP(*)
MATH_VEC3_OP(/)

template <typename T>
bool
operator==(Vector<T, 3> a, Vector<T, 3> b)
{
    return a.x == b.x && a.y == a.y && a.z == b.z;
}

MATH_VEC4_OP(+)
MATH_VEC4_OP(-)
MATH_VEC4_OP(*)
MATH_VEC4_OP(/)

template <typename T>
bool
operator==(Vector<T, 4> a, Vector<T, 4> b)
{
    return a.x == b.x && a.y == a.y && a.z == b.z && a.w == b.w;
}

template <typename T, size_t N>
bool
operator!=(Vector<T, N> a, Vector<T, N> b)
{
    return !(a == b);
}

MATH_VEC_OP_ASSIGN(+)
MATH_VEC_OP_ASSIGN(-)
MATH_VEC_OP_ASSIGN(*)
MATH_VEC_OP_ASSIGN(/)

//
// Matrix with M columns and N rows. Note that this is backward from convention in mathematics!
// Values are stored in column major order (OpenGL).
//    matrix[Column][Row]
//
template <typename T, size_t M, size_t N>
struct Matrix;

template <typename T, size_t M, size_t N>
bool
operator==(const Matrix<T, M, N> &a, const Matrix<T, M, N> &b) {
    for (size_t c = 0; c < Matrix<T, M, N>::COLUMNS; ++c) {
        if (a[c] != b[c]) return false;
    }
    return true;
}

template <typename T, size_t M, size_t N>
bool
operator!=(const Matrix<T, M, M> &a, const Matrix<T, M, N> &b) {
    return !(a == b);
}

template <typename T, size_t M, size_t N>
Matrix<T, M, N>
operator+(const Matrix<T, M, N> &a, const Matrix<T, M, N> &b) {
    Matrix<T, M, N> m;
    for (size_t c = 0; c < Matrix<T, M, N>::COLUMNS; ++c) {
        m[c] = a[c] + b[c];
    }
    return m;
}

template <typename T, size_t M, size_t N>
Matrix<T, M, N>
operator-(const Matrix<T, M, N> &a, const Matrix<T, M, N> &b) {
    Matrix<T, M, N> m;
    for (size_t c = 0; c < Matrix<T, M, N>::COLUMNS; ++c) {
        m[c] = a[c] - b[c];
    }
    return m;
}

template <typename T, size_t M, size_t N>
Matrix<T, M, N>
operator+=(Matrix<T, M, N> &a, const Matrix<T, M, N> &b) {
    return (a = a + b);
}

template <typename T, size_t M, size_t N>
Matrix<T, M, N>
operator-=(Matrix<T, M, N> &a, const Matrix<T, M, N> &b) {
    return (a = a - b);
}

template <typename T, size_t M, size_t N>
Matrix<T, M, N>
operator*=(Matrix<T, M, N> &a, const Matrix<T, M, N> &b)
{
    return (a = a * b);
}

template <typename T>
struct Matrix<T, 3, 3> {
    enum { COLUMNS = 3, ROWS = 3, SIZE = 3 };
    using col_t = Vector<T, 3>;
    using row_t = Vector<T, 3>;

    col_t values[3];

    Matrix() = default;
    constexpr Matrix(col_t c0, col_t c1, col_t c2)
        : values{c0, c1, c2} {}

    const col_t &operator[](size_t i) const { return values[i]; }
    col_t &operator[](size_t i) { return values[i]; }

    Matrix operator+() const { return *this; }
    Matrix operator-() const
    {
        return {-values[0], -values[1], -values[2]};
    }
};

template <typename T>
Matrix<T, 3, 3>
operator*(const Matrix<T, 3, 3> &a, const Matrix<T, 3, 3> &b)
{
    return Matrix<T, 3, 3>(
        a[0] * b[0][0] + a[1] * b[0][1] + a[2] * b[0][2],
        a[0] * b[1][0] + a[1] * b[1][1] + a[2] * b[1][2],
        a[0] * b[2][0] + a[1] * b[2][1] + a[2] * b[2][2]);
}

template <typename T>
Vector<T, 3>
operator*(const Matrix<T, 3, 3> &m, Vector<T, 3> v)
{
    return Vector<T, 3>(
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2]);
}
template <typename T>
Matrix<T, 3, 3>
transpose(const Matrix<T, 3, 3> &m)
{
    Matrix<T, 3, 3> m_t;
    m_t[0][0] = m[0][0];
    m_t[0][1] = m[1][0];
    m_t[0][2] = m[2][0];
    m_t[1][0] = m[0][1];
    m_t[1][1] = m[1][1];
    m_t[1][2] = m[2][1];
    m_t[2][0] = m[0][2];
    m_t[2][1] = m[1][2];
    m_t[2][2] = m[2][2];

    return m_t;
}

template <typename T>
Matrix<T, 3, 3>
inverse(const Matrix<T, 3, 3> &m)
{
    Matrix<T, 3, 3> m_i;
    T det = + m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
            - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
            + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    T det_i = T(1) / det;

    m_i[0][0] = +(m[1][1] * m[2][2] - m[1][2] * m[2][1]) * det_i;
    m_i[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * det_i;
    m_i[0][2] = +(m[0][1] * m[1][2] - m[0][2] * m[1][1]) * det_i;
    m_i[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * det_i;
    m_i[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * det_i;
    m_i[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * det_i;
    m_i[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * det_i;
    m_i[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * det_i;
    m_i[2][2] = +(m[0][0] * m[1][1] - m[0][1] * m[1][0]) * det_i;

    return m_i;
}

template <typename T>
struct Matrix<T, 4, 4> {
    enum { COLUMNS = 4, ROWS = 4, SIZE = 4 };

    using col_t = Vector<T, 4>;
    using row_t = Vector<T, 4>;

    col_t values[4];

    Matrix() = default;
    constexpr Matrix(col_t c0, col_t c1, col_t c2, col_t c3)
        : values{c0, c1, c2, c3} {}

    const col_t &operator[](size_t i) const { return values[i]; }
    col_t &operator[](size_t i) { return values[i]; }

    Matrix operator+() const { return *this; }
    Matrix operator-() const
    {
        return {-values[0], -values[1], -values[2], -values[3]};
    }
};

template <typename T>
Matrix<T, 4, 4>
operator*(const Matrix<T, 4, 4> &a, const Matrix<T, 4, 4> &b)
{
    return Matrix<T, 4, 4>(
        a[0] * b[0][0] + a[1] * b[0][1] + a[2] * b[0][2] + a[3] * b[0][3],
        a[0] * b[1][0] + a[1] * b[1][1] + a[2] * b[1][2] + a[3] * b[1][3],
        a[0] * b[2][0] + a[1] * b[2][1] + a[2] * b[2][2] + a[3] * b[2][3],
        a[0] * b[3][0] + a[1] * b[3][1] + a[2] * b[3][2] + a[3] * b[3][3]);
}

template <typename T>
Vector<T, 4>
operator*(const Matrix<T, 4, 4> &m, Vector<T, 4> v)
{
    return Vector<T, 4>(
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3],
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3]);
}

template <typename T>
Matrix<T, 4, 4>
transpose(const Matrix<T, 4, 4> &m)
{
    Matrix<T, 4, 4> m_t;
    m_t[0][0] = m[0][0];
    m_t[0][1] = m[1][0];
    m_t[0][2] = m[2][0];
    m_t[0][3] = m[3][0];
    m_t[1][0] = m[0][1];
    m_t[1][1] = m[1][1];
    m_t[1][2] = m[2][1];
    m_t[1][3] = m[3][1];
    m_t[2][0] = m[0][2];
    m_t[2][1] = m[1][2];
    m_t[2][2] = m[2][2];
    m_t[2][3] = m[3][2];
    m_t[3][0] = m[0][3];
    m_t[3][1] = m[1][3];
    m_t[3][2] = m[2][3];
    m_t[3][3] = m[3][3];

    return m_t;
}

template <typename T>
Matrix<T, 4, 4>
inverse(const Matrix<T, 4, 4> &m)
{
    Matrix<T, 4, 4> m_i;
    T c00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    T c01 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
    T c02 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
    T c03 = m[0][2] * m[3][3] - m[3][2] * m[0][3];
    T c04 = m[0][2] * m[2][3] - m[2][2] * m[0][3];
    T c05 = m[0][2] * m[1][3] - m[1][2] * m[0][3];
    T c06 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    T c07 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
    T c08 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
    T c09 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    T c10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
    T c11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    T c12 = m[0][1] * m[3][3] - m[3][1] * m[0][3];
    T c13 = m[0][1] * m[2][3] - m[2][1] * m[0][3];
    T c14 = m[0][1] * m[3][2] - m[3][1] * m[0][2];
    T c15 = m[0][1] * m[2][2] - m[2][1] * m[0][2];
    T c16 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
    T c17 = m[0][1] * m[1][2] - m[1][1] * m[0][2];

    T det = m[0][0] * (m[1][1] * c00 - m[2][1] * c01 + m[3][1] * c02)
          - m[1][0] * (m[0][1] * c00 - m[2][1] * c03 + m[3][1] * c04)
          + m[2][0] * (m[0][1] * c01 - m[1][1] * c03 + m[3][1] * c05)
          - m[3][0] * (m[0][1] * c02 - m[1][1] * c04 + m[2][1] * c05);

    T det_i = T(1) / det;

    m_i[0][0] = +(m[1][1] * c00 - m[2][1] * c01 + m[3][1] * c02) * det_i;
    m_i[0][1] = -(m[0][1] * c00 - m[2][1] * c03 + m[3][1] * c04) * det_i;
    m_i[0][2] = +(m[0][1] * c01 - m[1][1] * c03 + m[3][1] * c05) * det_i;
    m_i[0][3] = -(m[0][1] * c02 - m[1][1] * c04 + m[2][1] * c05) * det_i;
    m_i[1][0] = -(m[1][0] * c00 - m[2][0] * c01 + m[3][0] * c02) * det_i;
    m_i[1][1] = +(m[0][0] * c00 - m[2][0] * c03 + m[3][0] * c04) * det_i;
    m_i[1][2] = -(m[0][0] * c01 - m[1][0] * c03 + m[3][0] * c05) * det_i;
    m_i[1][3] = +(m[0][0] * c02 - m[1][0] * c04 + m[2][0] * c05) * det_i;
    m_i[2][0] = +(m[1][0] * c06 - m[2][0] * c07 + m[3][0] * c08) * det_i;
    m_i[2][1] = -(m[0][0] * c06 - m[2][0] * c12 + m[3][0] * c13) * det_i;
    m_i[2][2] = +(m[0][0] * c07 - m[1][0] * c12 + m[3][0] * c16) * det_i;
    m_i[2][3] = -(m[0][0] * c08 - m[1][0] * c13 + m[2][0] * c16) * det_i;
    m_i[3][0] = -(m[1][0] * c09 - m[2][0] * c10 + m[3][0] * c11) * det_i;
    m_i[3][1] = +(m[0][0] * c09 - m[2][0] * c14 + m[3][0] * c15) * det_i;
    m_i[3][2] = -(m[0][0] * c10 - m[1][0] * c14 + m[3][0] * c17) * det_i;
    m_i[3][3] = +(m[0][0] * c11 - m[1][0] * c15 + m[2][0] * c17) * det_i;
    return m_i;
}

// Inverts a Matrix4 assuming it is an affine transform.
// Avoids computing the full 4x4 inverse.
template <typename T>
Matrix<T, 4, 4>
affine_inverse(const Matrix<T, 4, 4> &m)
{
    Matrix<T, 3, 3> m3_i;
    m3_i[0] = Vector<T, 3>(m[0][0], m[0][1], m[0][2]);
    m3_i[1] = Vector<T, 3>(m[1][0], m[1][1], m[1][2]);
    m3_i[2] = Vector<T, 3>(m[2][0], m[2][1], m[2][2]);
    m3_i = inverse(m3_i);

    // Invert last column
    Vector<T, 3> c3_i = -m3_i * Vector<T, 3>(m[3][0], m[3][1], m[3][2]);

    Matrix<T, 4, 4> m_i;
    m_i[0] = Vector<T, 4>(m3_i[0][0], m3_i[0][1], m3_i[0][2], T(0));
    m_i[1] = Vector<T, 4>(m3_i[1][0], m3_i[1][1], m3_i[1][2], T(0));
    m_i[2] = Vector<T, 4>(m3_i[2][0], m3_i[2][1], m3_i[2][2], T(0));
    m_i[3] = Vector<T, 4>(c3_i[0], c3_i[1], c3_i[2], T(1));
    return m_i;
}

//
// Rect
//

template <typename T>
struct Rectangle {
    T x, y;
    T width, height;

    Rectangle() = default;
    constexpr Rectangle(T x_, T y_, T width_, T height_)
        : x(x_)
        , y(y_)
        , width(width_)
        , height(height_) {}
};

template <typename T>
bool
contains(Rectangle<T> r, Vector<T, 2> v)
{
    return v.x >= r.x && v.y >= r.y && v.x <= r.x + r.width && v.y <= r.y + r.height;
}

template <typename T>
bool
contains(Rectangle<T> a, Rectangle<T> b)
{
    return b.x >= a.x && b.y >= a.y && b.width <= a.width && b.height <= a.height;
}

template <typename T>
bool
intersects(Rectangle<T> a, Rectangle<T> b)
{
    return !(b.x > (a.x + a.width)
            || (b.x + b.width) < a.x
            || b.y > (a.y + a.height)
            || (b.y + b.height) < a.y);
}

//
// Common functions
//

template <typename T> T sin(T x) { return T(::sin(x)); }
template <typename T> T cos(T x) { return T(::cos(x)); }
template <typename T> T tan(T x) { return T(::tan(x)); }
template <typename T> T asin(T x) { return T(::asin(x)); }
template <typename T> T acos(T x) { return T(::acos(x)); }
template <typename T> T atan(T x) { return T(::atan(x)); }
template <typename T> T atan2(T y, T x) { return T(::atan2(y, x)); }

template <typename T> T sqrt(T x) { return T(::sqrt(x)); }
template <typename T> T floor(T x) { return T(::floor(x)); }
template <typename T> T ceil(T x) { return T(::ceil(x)); }
template <typename T> T round(T x) { return T(::round(x)); }
template <typename T> T abs(T x) { return T(::fabs(x)); }
template <typename T> T copysign(T x, T s) { return T(::copysign(x, s)); }

template <> inline float sin<float>(float x) { return (::sinf(x)); }
template <> inline float cos<float>(float x) { return (::cosf(x)); }
template <> inline float tan<float>(float x) { return (::tanf(x)); }
template <> inline float asin<float>(float x) { return (::asinf(x)); }
template <> inline float acos<float>(float x) { return (::acosf(x)); }
template <> inline float atan<float>(float x) { return (::atanf(x)); }
template <> inline float atan2<float>(float y, float x) { return ::atan2f(y, x); }

template <> inline float sqrt<float>(float x) { return (::sqrtf(x)); }
template <> inline float floor<float>(float x) { return (::floorf(x)); }
template <> inline float ceil<float>(float x) { return (::ceilf(x)); }
template <> inline float round<float>(float x) { return (::roundf(x)); }
template <> inline float abs<float>(float x) { return (::fabsf(x)); }
template <> inline float copysign<float>(float x, float s) { return ::copysignf(x, s); }

template <> inline int abs<int>(int x) { return (::abs(x)); }

template <typename T>
void
swap(T *a, T *b)
{
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

template <typename T>
bool
equal(T a, T b, T e = std::numeric_limits<T>::epsilon())
{
    return abs(a - b) <= e;
}

template <typename T>
T
min(T a, T b)
{
    return (b < a) ? b : a;
}

template <typename T>
T
max(T a, T b)
{
    return (a < b) ? b : a;
}

template <typename T>
T
sign(T x)
{
    return T(T(0) < x) - (x < T(0));
}

template <typename T>
T
clamp(T x, T a, T b)
{
    return min(max(x, a), b);
}

template <typename T>
T
saturate(T x)
{
    return clamp(x, T(0), T(1));
}

template <typename T>
T
lerp(T a, T b, T t)
{
    return a + (b - a) * t;
}

// Linear interpolation
template <typename T, size_t N, typename U>
Vector<T, N>
lerp(Vector<T, N> a, Vector<T, N> b, U t)
{
    return a + (b - a) * T(t);
}

// Spherical liner interpolation
template <typename T, size_t N, typename U>
Vector<T, N>
slerp(Vector<T, N> a, Vector<T, N> b, U t)
{
    T c_ht = dot(a, b);

    // theta == 0
    if (abs(c_ht) >= T(1)) {
        return a;
    }

    T s_ht = sqrt(T(1) - c_ht * c_ht);

    // For 180 degrees slerp is not defined
    if (abs(s_ht) < std::numeric_limits<T>::epsilon()) {
        return lerp(a, b, T(0.5));
    }

    T ht = acos(c_ht);
    T s_ht_i = T(1) / s_ht;
    T r1 = sin((T(1) - T(t)) * ht) * s_ht_i;
    T r2 = sin(T(t) * ht) * s_ht_i;

    return a * r1 + b * r2;
}

template <typename T>
int
floortoi(T x)
{
    return static_cast<int>(math::floor(x));
}

template <typename T, size_t N>
Vector<int, N>
floortoi(Vector<T, N> v)
{
    Vector<int, N> result;
    for (size_t i = 0; i < N; ++i) {
        result[i] = static_cast<int>(math::floor(v[i]));
    }

    return result;
}

template <typename T>
T
trunc(T x)
{
    return static_cast<T>(static_cast<int>(x));
}

template <typename T>
T
radians(T deg)
{
    return deg * T(0.01745329251994329);
}

template <typename T>
T
degrees(T rad)
{
    return rad * T(57.2957795130823208);
}

template <typename T>
T
dot(Vector<T, 2> a, Vector<T, 2> b)
{
    return a.x * b.x + a.y * b.y;
}

template <typename T>
T
dot(Vector<T, 3> a, Vector<T, 3> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
T
dot(Vector<T, 4> a, Vector<T, 4> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template <typename T, size_t N>
T
length(Vector<T, N> v)
{
    return sqrt(dot(v, v));
}

template <typename T, size_t N>
T
length2(Vector<T, N> v)
{
    return dot(v, v);
}

template <typename T, size_t N>
T
distance(Vector<T, N> a, Vector<T, N> b)
{
    return length(a - b);
}

template <typename T, size_t N>
Vector<T, N>
normalize(Vector<T, N> v)
{
    return v / length(v);
}

template <typename T, size_t N>
T
normalize(Vector<T, N> *v)
{
    T l = length(*v);
    *v /= l;
    return l;
}

template <typename T, size_t N>
bool
equal(Vector<T, N> a, Vector<T, N> b, T e = std::numeric_limits<T>::epsilon())
{
    return length2(a - b) <= e * e;
}

template <typename T, size_t M, size_t N>
bool
equal(const Matrix<T, M, N> &a, const Matrix<T, M, N> &b, T e)
{
    for (size_t c = 0; c < Matrix<T, M, N>::COLUMNS; ++c) {
        if (!equal(a[c], b[c], e)) {
            return false;
        }
    }
    return true;
}

template <typename T, size_t M, size_t N>
bool
equal(const Matrix<T, M, N> &a, const Matrix<T, M, N> &b)
{
    return equal<T, M, N>(a, b, std::numeric_limits<T>::epsilon());
}

template <typename T, size_t N>
Vector<T, N>
reflect(Vector<T, N> I, Vector<T, N> n)
{
    return I - n * dot(n, I) * T(2);
}

template <typename T>
Vector<T, 3>
cross(Vector<T, 3> a, Vector<T, 3> b)
{
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

template <size_t X, size_t Y, typename T, size_t N>
Vector<T, 2>
shuffle(Vector<T, N> v)
{
    return Vector<T, 2>(v[X], v[Y]);
}

template <size_t X, size_t Y, size_t Z, typename T, size_t N>
Vector<T, 3>
shuffle(Vector<T, N> v)
{
    return Vector<T, 3>(v[X], v[Y], v[Z]);
}

template <size_t X, size_t Y, size_t Z, size_t W, typename T, size_t N>
Vector<T, 4>
shuffle(Vector<T, N> v)
{
    return Vector<T, 4>(v[X], v[Y], v[Z], v[W]);
}

//
// Broadcast functions
//

#define MATH_VEC_BROADCAST_FN(fn)          \
    template <typename T, size_t N>        \
    Vector<T, N>                           \
    fn(Vector<T, N> v)                     \
    {                                      \
        Vector<T, N> result;               \
        for (size_t i = 0; i < N; ++i) {   \
            result[i] = math::fn(v[i]);    \
        }                                  \
                                           \
        return result;                     \
    }

MATH_VEC_BROADCAST_FN(floor)
MATH_VEC_BROADCAST_FN(trunc)
MATH_VEC_BROADCAST_FN(round)
MATH_VEC_BROADCAST_FN(ceil)
MATH_VEC_BROADCAST_FN(radians)
MATH_VEC_BROADCAST_FN(degrees)

#undef MATH_VEC_BROADCAST_FN

//
// Transform
//

template <typename T>
Matrix<T, 4, 4>
perspective(T fovy, T aspect, T znear, T zfar)
{
    T h = tan(fovy / T(2));

    Matrix<T, 4, 4> m{};
    m[0][0] = T(1) / (aspect * h);
    m[1][1] = T(1) / h;
    m[2][2] = -(zfar + znear) / (zfar - znear);
    m[2][3] = -T(1);
    m[3][2] = -(T(2) * zfar * znear) / (zfar - znear);
    return m;
}

// Right handed
template <typename T>
Matrix<T, 4, 4>
lookat(Vector<T, 3> eye, Vector<T, 3> target, Vector<T, 3> up)
{
    Vector<T, 3> d = normalize(target - eye);
    Vector<T, 3> r = normalize(cross(d, up));
    Vector<T, 3> u = normalize(cross(r, d));

    Matrix<T, 4, 4> m;
    m[0] = Vector<T, 4>(r.x, u.x, -d.x, T(0)),
    m[1] = Vector<T, 4>(r.y, u.y, -d.y, T(0)),
    m[2] = Vector<T, 4>(r.z, u.z, -d.z, T(0)),
    m[3] = Vector<T, 4>(-dot(r, eye), -dot(u, eye), dot(d, eye), T(1));

    return m;
}

template <typename T>
Matrix<T, 4, 4>
translation(Vector<T, 3> v)
{
    Matrix<T, 4, 4> m;
    m[0] = Vector<T, 4>(  1,   0,   0, 0);
    m[1] = Vector<T, 4>(  0,   1,   0, 0);
    m[2] = Vector<T, 4>(  0,   0,   1, 0);
    m[3] = Vector<T, 4>(v.x, v.y, v.z, 1);

    return m;
}

// Creates a rotation matrix from a quaternion
template <typename T>
Matrix<T, 4, 4>
rotation(Vector<T, 4> q)
{
    T xx = q.x * q.x;
    T xy = q.x * q.y;
    T xz = q.x * q.z;
    T xw = q.x * q.w;
    T yy = q.y * q.y;
    T yz = q.y * q.z;
    T yw = q.y * q.w;
    T zz = q.z * q.z;
    T zw = q.z * q.w;

    Matrix<T, 4, 4> m;
    m[0][0] = 1 - 2 * (yy + zz);
    m[0][1] =     2 * (xy + zw);
    m[0][2] =     2 * (xz - yw);
    m[0][3] = 0;

    m[1][0] =     2 * (xy - zw);
    m[1][1] = 1 - 2 * (xx + zz);
    m[1][2] =     2 * (yz + xw);
    m[1][3] = 0;

    m[2][0] =     2 * (xz + yw);
    m[2][1] =     2 * (yz - xw);
    m[2][2] = 1 - 2 * (xx + yy);
    m[2][3] = 0;

    m[3] = Vector<T, 4>{0, 0, 0, 1};

    return m;
}

template <typename T>
Matrix<T, 4, 4>
scale(Vector<T, 3> v)
{
    Matrix<T, 4, 4> m;
    m[0] = Vector<T, 4>(v.x,   0,   0,   0);
    m[1] = Vector<T, 4>(  0, v.y,   0,   0);
    m[2] = Vector<T, 4>(  0,   0, v.z,   0);
    m[3] = Vector<T, 4>(  0,   0,   0,   1);

    return m;
}

//
// Quaternion
//

// Returns the quaternion identity
inline Vector<float, 4>
quat()
{
    return Vector<float, 4>(0, 0, 0, 1);
}

// Constructs a quaternion by applying a rotation to an axis
template <typename T>
Vector<T, 4>
quat(Vector<T, 3> axis, T theta)
{
    T ct = cos(theta * T(0.5));
    T st = sin(theta * T(0.5));
    axis *= st;

    Vector<T, 4> q;
    q.x = axis.x;
    q.y = axis.y;
    q.z = axis.z;
    q.w = ct;

    return q;
}

// Returns the axis of rotation.
// Use quat_angle to get the angle of rotation applied to the axis.
template <typename T>
Vector<T, 3>
quat_axis(Vector<T, 4> q)
{
    T det = sqrt(T(1) - q.w * q.w);

    if (det != T(0)) {
        T det_r = T(1) / det;
        return Vector<T, 3>(q.x * det_r, q.y * det_r, q.z * det_r);
    }

    // Arbitrary normalized axis
    return Vector<T, 3>{0, 0, 1};
}

// Returns the angle of rotation.
// Use quat_axis to get the axis which the rotation is applied.
template <typename T>
T
quat_angle(Vector<T, 4> q)
{
    return 2 * acos(q.w);
}

// Constructs a quaternion from euler angles
// X=roll, Y=pitch, Z=yaw
// Rotation applied in order yaw->pitch->roll (ZYX)
template <typename T>
Vector<T, 4>
quat(Vector<T, 3> euler)
{
    euler *= T(0.5);
    T cr = cos(euler.x);
    T sr = sin(euler.x);
    T cp = cos(euler.y);
    T sp = sin(euler.y);
    T cy = cos(euler.z);
    T sy = sin(euler.z);

    Vector<T, 4> q;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    q.w = cr * cp * cy + sr * sp * sy;

    return q;
}

// Euler angles from quaternion
// X=roll, Y=pitch, Z=yaw
template <typename T>
Vector<T, 3>
euler(Vector<T, 4> q)
{
    T sr_cp =     2 * (q.w * q.x + q.y * q.z);
    T cr_cp = 1 - 2 * (q.x * q.x + q.y * q.y);
    T sp    =     2 * (q.w * q.y - q.z * q.x);
    T sy_cp =     2 * (q.w * q.z + q.x * q.y);
    T cy_cp = 1 - 2 * (q.y * q.y + q.z * q.z);

    Vector<T, 3> r;
    r.x = atan2(sr_cp, cr_cp);
    r.y = (abs(sp) >= 1) ? copysign(PI_2, sp) : asin(sp); // asin range [-PI_2, PI_2]
    r.z = atan2(sy_cp, cy_cp);
    return r;
}

// Quaterion conjugate
template <typename T>
Vector<T, 4>
conjugate(Vector<T, 4> q)
{
    return Vector<T, 4>(-q.x, -q.y, -q.z, q.w);
}

// Quaternion multiply
template <typename T>
Vector<T, 4>
qmul(Vector<T, 4> q1, Vector<T, 4> q2)
{
    Vector<T, 4> q;
    q.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    q.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    q.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    q.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    return q;
}

typedef Vector<float, 2> Vector2;
typedef Vector<float, 3> Vector3;
typedef Vector<float, 4> Vector4;

typedef Vector<int, 2> Point2;
typedef Vector<int, 3> Point3;
typedef Vector<int, 4> Point4;

typedef Matrix<float, 3, 3> Matrix3;
typedef Matrix<float, 4, 4> Matrix4;

typedef Vector<float, 4> Quaternion;

typedef Rectangle<float> Quad;
typedef Rectangle<int> Rect;

inline constexpr Vector2 vec2(float s) { return {s, s}; }
inline constexpr Vector2 vec2(float x, float y) { return {x, y}; }
inline constexpr Vector2 vec2(Vector3 xy) { return {xy.x, xy.y}; }
inline constexpr Vector2 vec2(Vector4 xy) { return {xy.x, xy.y}; }

inline constexpr Vector3 vec3(float s) { return {s, s, s}; }
inline constexpr Vector3 vec3(float x, float y, float z) { return {x, y, z}; }
inline constexpr Vector3 vec3(Vector2 xy) { return {xy.x, xy.y, 0.0f}; }
inline constexpr Vector3 vec3(Vector2 xy, float z) { return {xy.x, xy.y, z}; }
inline constexpr Vector3 vec3(Vector4 xyz) { return {xyz.x, xyz.y, xyz.z}; }

inline constexpr Vector4 vec4(float s) { return {s, s, s, s}; }
inline constexpr Vector4 vec4(float x, float y, float z, float w) { return {x, y, z, w}; }
inline constexpr Vector4 vec4(Vector2 xy) { return {xy.x, xy.y, 0.0f, 0.0f}; }
inline constexpr Vector4 vec4(Vector2 xy, Vector2 zw) { return {xy.x, xy.y, zw.x, zw.y}; }
inline constexpr Vector4 vec4(Vector2 xy, float z, float w) { return {xy.x, xy.y, z, w}; }
inline constexpr Vector4 vec4(Vector3 xyz) { return {xyz.x, xyz.y, xyz.z, 0.0f}; }
inline constexpr Vector4 vec4(Vector3 xyz, float w) { return {xyz.x, xyz.y, xyz.z, w}; }

inline constexpr Point2 point2(int s) { return {s, s}; }
inline constexpr Point2 point2(int x, int y) { return {x, y}; }
inline constexpr Point2 point2(Point3 xy) { return {xy.x, xy.y}; }
inline constexpr Point2 point2(Point4 xy) { return {xy.x, xy.y}; }

inline constexpr Point3 point3(int s) { return {s, s, s}; }
inline constexpr Point3 point3(int x, int y, int z) { return {x, y, z}; }
inline constexpr Point3 point3(Point2 xy) { return {xy.x, xy.y, 0}; }
inline constexpr Point3 point3(Point2 xy, int z) { return {xy.x, xy.y, z}; }
inline constexpr Point3 point3(Point4 xyz) { return {xyz.x, xyz.y, xyz.z}; }

inline constexpr Point4 point4(int s) { return {s, s, s, s}; }
inline constexpr Point4 point4(int x, int y, int z, int w) { return {x, y, z, w}; }
inline constexpr Point4 point4(Point2 xy) { return {xy.x, xy.y, 0, 0}; }
inline constexpr Point4 point4(Point2 xy, Point2 zw) { return {xy.x, xy.y, zw.x, zw.y}; }
inline constexpr Point4 point4(Point2 xy, int z, int w) { return {xy.x, xy.y, z, w}; }
inline constexpr Point4 point4(Point3 xyz) { return {xyz.x, xyz.y, xyz.z, 0}; }
inline constexpr Point4 point4(Point3 xyz, int w) { return {xyz.x, xyz.y, xyz.z, w}; }

inline constexpr Quad quad(float x, float y, float w, float h) { return {x, y, w, h}; }
inline constexpr Quad quad(Vector2 xy, float w, float h) { return {xy.x, xy.y, w, h}; }
inline constexpr Quad quad(Vector2 xy, Vector2 size) { return {xy.x, xy.y, size.x, size.y}; }

inline constexpr Rect rect(int x, int y, int w, int h) { return {x, y, w, h}; }
inline constexpr Rect rect(Point2 xy, int w, int h) { return {xy.x, xy.y, w, h}; }
inline constexpr Rect rect(Point2 xy, Point2 size) { return {xy.x, xy.y, size.x, size.y}; }

inline constexpr Matrix3
mat3(float s)
{
    return {Vector3(s, 0, 0),
            Vector3(0, s, 0),
            Vector3(0, 0, s)};
}

inline constexpr Matrix3
mat3(Vector3 c0, Vector3 c1, Vector3 c2)
{
    return {c0, c1, c2};
}

inline constexpr Matrix4
mat4(float s)
{
    return {Vector4(s, 0, 0, 0),
            Vector4(0, s, 0, 0),
            Vector4(0, 0, s, 0),
            Vector4(0, 0, 0, s)};
}

inline constexpr Matrix4
mat4(Vector4 c0, Vector4 c1, Vector4 c2, Vector4 c3)
{
    return {c0, c1, c2, c3};
}

} // namespace math

#ifndef MATH_NO_GLOBAL_NAMESPACE

using math::PI;
using math::PI_2;
using math::PI_4;
using math::TAU;
using math::SQRT2;

using math::Vector2;
using math::Vector3;
using math::Vector4;

using math::Point2;
using math::Point3;
using math::Point4;

using math::Matrix3;
using math::Matrix4;

using math::Quaternion;

using math::vec2;
using math::vec3;
using math::vec4;

using math::point2;
using math::point3;
using math::point4;

using math::mat3;
using math::mat4;

#endif // MATH_NO_GLOBAL_NAMESPACE

#undef MATH_VEC2_OP
#undef MATH_VEC3_OP
#undef MATH_VEC4_OP
#undef MATH_VEC_OP_ASSIGN

#endif // CORE_MATH_H_
