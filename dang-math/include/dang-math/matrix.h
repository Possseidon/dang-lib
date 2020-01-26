#pragma once

#include "global.h"

#include "vector.h"
#include "bounds.h"

namespace dang::math
{

template <typename T, std::size_t Cols, std::size_t Rows>
class Matrix;

template <typename T, std::size_t Cols, std::size_t Rows>
class Matrix : protected std::array<Vector<T, Rows>, Cols> {
public:
    inline constexpr Matrix() : std::array<Vector<T, Rows>, Cols>() {}

    inline constexpr Matrix(T value) : std::array<Vector<T, Rows>, Cols>()
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = value;
    }

    inline constexpr Matrix(const std::array<T, Rows>(&values)[Cols]) : std::array<Vector<T, Rows>, Cols>()
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = values[i];
    }

    static constexpr Matrix identity()
    {
        Matrix result;
        for (std::size_t i = 0; i < (Cols < Rows ? Cols : Rows); i++)
            result(i, i) = T(1);
        return result;
    }

    inline constexpr Vector<T, Rows>& operator[](std::size_t col)
    {
        return std::array<Vector<T, Rows>, Cols>::operator[](col);
    }

    inline constexpr const Vector<T, Rows>& operator[](std::size_t col) const
    {
        return std::array<Vector<T, Rows>, Cols>::operator[](col);
    }

    inline constexpr T& operator[](const dmath::svec2& pos)
    {
        return (*this)(pos.x(), pos.y());
    }

    inline constexpr const T& operator[](const dmath::svec2& pos) const
    {
        return (*this)(pos.x(), pos.y());
    }

    inline constexpr T& operator()(std::size_t col, std::size_t row)
    {
        return (*this)[col][row];
    }

    inline constexpr const T operator()(std::size_t col, std::size_t row) const
    {
        return (*this)[col][row];
    }

    inline constexpr const Matrix<T, Cols, Rows>& operator+() const
    {
        return *this;
    }

    inline constexpr const Matrix<T, Cols, Rows> operator-() const
    {
        return unary([](Vector<T, Rows> a) { return -a; });
    }

#define DMATH_MATRIX_OPERATION(op) \
    friend inline constexpr Matrix<T, Cols, Rows> operator op(Matrix<T, Cols, Rows> lhs, const Matrix<T, Cols, Rows>& rhs) \
    { return lhs op ## = rhs; } \
    friend inline constexpr Matrix<T, Cols, Rows>& operator op ## =(Matrix<T, Cols, Rows>& lhs, const Matrix<T, Cols, Rows>& rhs) \
    { return assignment(lhs, rhs, [](Vector<T, Rows>& a, Vector<T, Rows> b) { a op ## = b; }); }

    DMATH_MATRIX_OPERATION(+);
    DMATH_MATRIX_OPERATION(-);

#undef DMATH_MATRIX_OPERATION

    template <std::size_t OtherCols>
    friend inline constexpr Matrix<T, OtherCols, Rows> operator*(const Matrix<T, Cols, Rows>& lhs, const Matrix<T, OtherCols, Cols>& rhs)
    {
        Matrix<T, OtherCols, Rows> result;
        for (const auto& pos : dmath::sbounds2 { { OtherCols, Rows } })
            for (std::size_t i = 0; i < Cols; i++)
                result[pos] += lhs(i, pos.y()) * rhs(pos.x(), i);
        return result;
    }

    template <std::size_t OtherCols>
    friend inline constexpr Matrix<T, Cols, Rows> operator/(Matrix<T, Cols, Rows> lhs, const Matrix<T, OtherCols, Cols>& rhs)
    {
        return lhs * rhs.inverse();
    }

#define DMATH_MATRIX_COMPARE(merge, op) \
    friend inline constexpr bool operator op(const Matrix<T, Cols, Rows>& lhs, const Matrix<T, Cols, Rows>& rhs) \
    { return lhs.merge(rhs, [](Vector<T, Rows> a, Vector<T, Rows> b) { return a op b; }); }

    DMATH_MATRIX_COMPARE(all, == );
    DMATH_MATRIX_COMPARE(any, != );
    DMATH_MATRIX_COMPARE(all, < );
    DMATH_MATRIX_COMPARE(all, <= );
    DMATH_MATRIX_COMPARE(all, > );
    DMATH_MATRIX_COMPARE(all, >= );

#undef DMATH_MATRIX_COMPARE

private:
    template <typename Op>
    static inline constexpr Matrix<T, Cols, Rows>& assignment(Matrix<T, Cols, Rows>& lhs, const Matrix<T, Cols, Rows>& rhs, const Op& op)
    {
        for (std::size_t i = 0; i < Cols; i++)
            op(lhs[i], rhs[i]);
        return lhs;
    }

    template <typename Op>
    inline constexpr Matrix<T, Cols, Rows> binary(const Matrix<T, Cols, Rows>& other, const Op& op) const
    {
        Matrix<T, Cols, Rows> result;
        for (std::size_t i = 0; i < Cols; i++)
            result[i] = op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    inline constexpr Matrix<T, Cols, Rows> unary(const Op& op) const
    {
        Matrix<T, Cols, Rows> result;
        for (std::size_t i = 0; i < Cols; i++)
            result[i] = op((*this)[i]);
        return result;
    }

    template <typename Op>
    inline constexpr bool all(const Matrix<T, Cols, Rows>& other, const Op& op) const
    {
        bool result = true;
        for (std::size_t i = 0; i < Cols; i++)
            result = result && op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    inline constexpr bool any(const Matrix<T, Cols, Rows>& other, const Op& op) const
    {
        bool result = false;
        for (std::size_t i = 0; i < Cols; i++)
            result = result || op((*this)[i], other[i]);
        return result;
    }
};

template <std::size_t Cols, std::size_t Rows>
using mat = Matrix<float, Cols, Rows>;

template <std::size_t Cols, std::size_t Rows>
using dmat = Matrix<double, Cols, Rows>;

template <std::size_t Cols, std::size_t Rows>
using imat = Matrix<int, Cols, Rows>;

template <std::size_t Cols, std::size_t Rows>
using umat = Matrix<unsigned, Cols, Rows>;

template <std::size_t Cols, std::size_t Rows>
using smat = Matrix<std::size_t, Cols, Rows>;

#define DMATH_MATRIX_DEFINE(mtype) \
using mtype ## 1x1 = mtype<1, 1>; \
using mtype ## 1x2 = mtype<1, 2>; \
using mtype ## 1x3 = mtype<1, 3>; \
using mtype ## 1x4 = mtype<1, 4>; \
using mtype ## 2x1 = mtype<2, 1>; \
using mtype ## 2x2 = mtype<2, 2>; \
using mtype ## 2x3 = mtype<2, 3>; \
using mtype ## 2x4 = mtype<2, 4>; \
using mtype ## 3x1 = mtype<3, 1>; \
using mtype ## 3x2 = mtype<3, 2>; \
using mtype ## 3x3 = mtype<3, 3>; \
using mtype ## 3x4 = mtype<3, 4>; \
using mtype ## 4x1 = mtype<4, 1>; \
using mtype ## 4x2 = mtype<4, 2>; \
using mtype ## 4x3 = mtype<4, 3>; \
using mtype ## 4x4 = mtype<4, 4>; \
using mtype ## 1 = mtype ## 1x1; \
using mtype ## 2 = mtype ## 2x2; \
using mtype ## 3 = mtype ## 3x3; \
using mtype ## 4 = mtype ## 4x4; 

DMATH_MATRIX_DEFINE(mat)
DMATH_MATRIX_DEFINE(dmat)
DMATH_MATRIX_DEFINE(imat)
DMATH_MATRIX_DEFINE(umat)
DMATH_MATRIX_DEFINE(smat)

#undef DMATH_MATRIX_DEFINE

}
