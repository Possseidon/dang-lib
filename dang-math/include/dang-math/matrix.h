#pragma once

#include "global.h"

#include "vector.h"

namespace dang::math
{

template <typename T, std::size_t Cols, std::size_t Rows>
class Matrix;

template <typename T, std::size_t Cols, std::size_t Rows>
class Matrix : protected std::array<Vector<T, Rows>, Cols> {
public:
    inline constexpr Matrix() = default;
    inline constexpr Matrix(std::array<Vector<T, Rows>, Cols> values) : std::array<Vector<T, Rows>, Cols>(values) {}

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

    inline constexpr T& operator()(std::size_t col, std::size_t row)
    {
        return (*this)[col][row];
    }

    inline constexpr const T operator()(std::size_t col, std::size_t row) const
    {
        return (*this)[col][row];
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

#define DEFINE_MATRIX(mtype) \
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

DEFINE_MATRIX(mat)
DEFINE_MATRIX(dmat)
DEFINE_MATRIX(imat)
DEFINE_MATRIX(umat)
DEFINE_MATRIX(smat)

#undef DEFINE_MATRIX

}
