#pragma once

#include "utils.h"        
#include "vector.h"
#include "bounds.h"

namespace dang::math
{

template <typename T, std::size_t Cols, std::size_t Rows = Cols>
struct Matrix : protected std::array<Vector<T, Rows>, Cols> {
    using Base = std::array<Vector<T, Rows>, Cols>;

    inline constexpr Matrix() : Base() {}

    inline constexpr Matrix(T value) : Base()
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = value;
    }

    inline constexpr Matrix(const std::array<T, Rows>(&columns)[Cols]) : Base()
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = columns[i];
    }

    inline constexpr Matrix(Vector<T, Rows> col) : Base({ col })
    {
        static_assert(Cols == 1, "only mat1xN can be constructed from vector");
    }

    static inline constexpr Matrix identity()
    {
        Matrix result;
        for (std::size_t i = 0; i < (Cols < Rows ? Cols : Rows); i++)
            result(i, i) = T(1);
        return result;
    }

    inline constexpr operator Vector<T, Rows>() const
    {
        static_assert(Cols == 1, "only mat1xN can be converted to vector");
        return (*this)[0];
    }

    inline constexpr operator T() const
    {
        static_assert(Cols == 1 && Rows == 1, "only mat1x1 can be converted to value type");
        return (*this)(0, 0);
    }

    inline constexpr Vector<T, Rows>& operator[](std::size_t col)
    {
        return Base::operator[](col);
    }

    inline constexpr const Vector<T, Rows>& operator[](std::size_t col) const
    {
        return Base::operator[](col);
    }

    template <std::size_t StartCol, std::size_t StartRow, std::size_t ColCount, std::size_t RowCount>
    inline constexpr Matrix<T, ColCount, RowCount> subMatrix() const
    {
        Matrix<T, ColCount, RowCount> result;
        for (auto [col, row] : sbounds2(svec2(ColCount, RowCount)))
            result(col, row) = (*this)(StartCol + col, StartRow + row);
        return result;
    }

    template <std::size_t StartCol, std::size_t StartRow, std::size_t ColCount, std::size_t RowCount>
    inline constexpr void setSubMatrix(Matrix<T, ColCount, RowCount> matrix)
    {
        for (auto [col, row] : sbounds2(svec2(ColCount, RowCount)))
            (*this)(StartCol + col, StartRow + row) = matrix(col, row);
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

    inline constexpr const T& operator()(std::size_t col, std::size_t row) const
    {
        return (*this)[col][row];
    }

    inline constexpr Matrix<T, Rows, Cols> transpose()
    {
        Matrix<T, Rows, Cols> result;
        for (const auto& pos : dmath::sbounds2{ {Cols, Rows} })
            result(pos.y(), pos.x()) = (*this)[pos];
        return result;
    }

    inline constexpr Matrix<T, Cols - 1, Rows - 1> minor(std::size_t col, std::size_t row) const
    {
        return minor({ col, row });
    }

    inline constexpr Matrix<T, Cols - 1, Rows - 1> minor(const dmath::svec2& pos) const
    {
        Matrix<T, Cols - 1, Rows - 1> result;
        std::size_t rcol = 0;
        for (std::size_t col = 0; col < Cols; col++) {
            if (col == pos.x())
                continue;
            std::size_t rrow = 0;
            for (std::size_t row = 0; row < Rows; row++) {
                if (row == pos.y())
                    continue;
                result(rcol, rrow) = (*this)(col, row);
                rrow++;
            }
            rcol++;
        }
        return result;
    }

    inline constexpr T cofactor(std::size_t col, std::size_t row) const
    {
        static_assert(std::is_signed_v<T>, "mat::cofactor requires a signed value");
        return cofactor({ col, row });
    }

    inline constexpr T cofactor(const dmath::svec2& pos) const
    {
        static_assert(std::is_signed_v<T>, "mat::cofactor requires a signed value");
        const T factor = T(1) - ((pos.x() + pos.y()) & 1) * 2;
        return minor(pos).determinant() * factor;
    }

    inline constexpr Matrix<T, Cols, Rows> cofactorMatrix() const
    {
        static_assert(std::is_signed_v<T>, "mat::cofactorMatrix requires a signed value");
        Matrix<T, Cols, Rows> result;
        for (const auto& pos : dmath::sbounds2{ {Cols, Rows} })
            result[pos] = cofactor(pos);
        return result;
    }

    inline constexpr Matrix<T, Rows, Cols> adjugate() const
    {
        static_assert(std::is_signed_v<T>, "mat::adjugate requires a signed value");
        if constexpr (Rows == 1 && Cols == 1)
            return identity();
        else
            return cofactorMatrix().transpose();
    }

    inline constexpr std::optional<Matrix<T, Rows, Cols>> inverse() const
    {
        static_assert(std::is_floating_point_v<T>, "mat::inverse requires a floating point type");
        static_assert(Rows == Cols, "mat::inverse requires a square matrix");

        constexpr std::size_t Dim = Cols;
        constexpr std::size_t DimHalf1 = Dim / 2 + Dim % 2;
        constexpr std::size_t DimHalf2 = Dim / 2;

        if constexpr (Dim <= 4) {
            T det = determinant();
            if (det == T())
                return std::nullopt;
            return adjugate() / det;
        }
        else {
            auto a = subMatrix<0, 0, DimHalf1, DimHalf1>();
            auto b = subMatrix<DimHalf1, 0, DimHalf2, DimHalf1>();
            auto c = subMatrix<0, DimHalf1, DimHalf1, DimHalf2>();
            auto d = subMatrix<DimHalf1, DimHalf1, DimHalf2, DimHalf2>();

            auto a_inv = a.inverse();
            if (!a_inv)
                return std::nullopt;

            auto s_inv = (d - c * *a_inv * b).inverse();
            if (!s_inv)
                return std::nullopt;

            auto e = *a_inv + *a_inv * b * *s_inv * c * *a_inv;
            auto f = -*a_inv * b * *s_inv;
            auto g = -*s_inv * c * *a_inv;
            auto h = *s_inv;

            Matrix<T, Dim> result;
            result.setSubMatrix<0, 0, DimHalf1, DimHalf1>(e);
            result.setSubMatrix<DimHalf1, 0, DimHalf2, DimHalf1>(f);
            result.setSubMatrix<0, DimHalf1, DimHalf1, DimHalf2>(g);
            result.setSubMatrix<DimHalf1, DimHalf1, DimHalf2, DimHalf2>(h);
            return result;
        }
    }

    inline constexpr T determinant() const
    {
        static_assert(std::is_floating_point_v<T>, "mat::determinant requires a floating point type");
        constexpr std::size_t Dim = Cols < Rows ? Cols : Rows;
        if constexpr (Dim == 1) {
            return *this;
        }
        else if constexpr (Dim == 2) {
            return
                (*this)(0, 0) * (*this)(1, 1) -
                (*this)(0, 1) * (*this)(1, 0);
        }
        else if constexpr (Dim == 3) {
            return
                (*this)(0, 0) * (*this)(1, 1) * (*this)(2, 2) +
                (*this)(0, 1) * (*this)(1, 2) * (*this)(2, 0) +
                (*this)(0, 2) * (*this)(1, 0) * (*this)(2, 1) -
                (*this)(2, 0) * (*this)(1, 1) * (*this)(0, 2) -
                (*this)(2, 1) * (*this)(1, 2) * (*this)(0, 0) -
                (*this)(2, 2) * (*this)(1, 0) * (*this)(0, 1);
        }
        else {
            T result = T();
            for (std::size_t i = 0; i < Dim; i++)
                result += (*this)(i, 0) * cofactor(i, 0);
            return result;
        }
    }

    inline constexpr bool solvable() const
    {
        return determinant() != T();
    }

    inline constexpr std::optional<T> solveCol(std::size_t col)
    {
        static_assert(Cols == Rows + 1, "mat::solveCol() requires a single extra column");

        if constexpr (Rows >= 6) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return (*inv * (*this)[Rows])[col];
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            auto tmp = (*this)[col];
            (*this)[col] = (*this)[Rows];
            (*this)[Rows] = tmp;

            T result = determinant() / oldDeterminant;

            (*this)[Rows] = (*this)[col];
            (*this)[col] = tmp;

            return result;
        }
    }

    inline constexpr std::optional<T> solveCol(std::size_t col) const
    {
        static_assert(Cols == Rows + 1, "mat::solveCol() requires a single extra column");

        if constexpr (Rows >= 6) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return (*inv * (*this)[Rows])[col];
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            auto swappedMatrix = *this;
            swappedMatrix[col] = (*this)[Rows];
            swappedMatrix[Rows] = (*this)[col];
            return swappedMatrix.determinant() / oldDeterminant;
        }
    }

    inline constexpr std::optional<T> solveCol(std::size_t col, Vector<T, Cols> vector)
    {
        static_assert(Cols == Rows, "mat::solveCol(vector) requires a square matrix");

        if constexpr (Rows >= 6) {
            if (auto inv = inverse())
                return (*inv * vector)[col];
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            auto tmp = (*this)[col];
            (*this)[col] = vector;
            vector = tmp;

            T result = determinant() / oldDeterminant;

            vector = (*this)[col];
            (*this)[col] = tmp;

            return result;
        }
    }

    inline constexpr std::optional<T> solveCol(std::size_t col, Vector<T, Cols> vector) const
    {
        static_assert(Cols == Rows, "mat::solveCol(vector) requires a square matrix");

        if constexpr (Rows >= 6) {
            if (auto inv = inverse())
                return (*inv * vector)[col];
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            auto swappedMatrix = *this;
            swappedMatrix[col] = vector;
            return swappedMatrix.determinant() / oldDeterminant;
        }
    }

    inline constexpr std::optional<Vector<T, Rows>> solve()
    {
        static_assert(Cols == Rows + 1, "mat::solve() requires a single extra column");

        if constexpr (Rows >= 5) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return *inv * (*this)[Rows];
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            Vector<T, Rows> result;

            for (std::size_t col = 0; col < Rows; col++) {
                auto tmp = (*this)[col];
                (*this)[col] = (*this)[Rows];
                (*this)[Rows] = tmp;

                result[col] = determinant() / oldDeterminant;

                (*this)[Rows] = (*this)[col];
                (*this)[col] = tmp;
            }

            return result;
        }
    }

    inline constexpr std::optional<Vector<T, Rows>> solve() const
    {
        static_assert(Cols == Rows + 1, "mat::solve() requires a single extra column");

        if constexpr (Rows >= 5) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return *inv * (*this)[Rows];
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            Vector<T, Rows> result;
            auto swappedMatrix = *this;

            for (std::size_t col = 0; col < Rows; col++) {
                auto tmp = swappedMatrix[col];
                swappedMatrix[col] = swappedMatrix[Rows];
                swappedMatrix[Rows] = tmp;

                result[col] = swappedMatrix.determinant() / oldDeterminant;

                swappedMatrix[Rows] = swappedMatrix[col];
                swappedMatrix[col] = tmp;
            }

            return result;
        }
    }

    inline constexpr std::optional<Vector<T, Cols>> solve(Vector<T, Cols> vector)
    {
        static_assert(Cols == Rows, "mat::solve(vector) requires a square matrix");

        if constexpr (Rows >= 5) {
            if (auto inv = inverse())
                return *inv * vector;
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            Vector<T, Cols> result;

            for (std::size_t col = 0; col < Cols; col++) {
                auto tmp = (*this)[col];
                (*this)[col] = vector;
                vector = tmp;

                result[col] = determinant() / oldDeterminant;

                vector = (*this)[col];
                (*this)[col] = tmp;
            }

            return result;
        }
    }

    inline constexpr std::optional<Vector<T, Cols>> solve(Vector<T, Cols> vector) const
    {
        static_assert(Cols == Rows, "mat::solve(vector) requires a square matrix");

        if constexpr (Rows >= 5) {
            if (auto inv = inverse())
                return *inv * vector;
            return std::nullopt;
        }
        else {
            T oldDeterminant = determinant();
            if (oldDeterminant == T())
                return std::nullopt;

            Vector<T, Cols> result;
            auto swappedMatrix = *this;

            for (std::size_t col = 0; col < Cols; col++) {
                auto tmp = swappedMatrix[col];
                swappedMatrix[col] = vector;
                vector = tmp;

                result[col] = swappedMatrix.determinant() / oldDeterminant;

                vector = swappedMatrix[col];
                swappedMatrix[col] = tmp;
            }

            return result;
        }
    }

    inline constexpr const Matrix<T, Cols, Rows>& operator+() const
    {
        return *this;
    }

    inline constexpr Matrix<T, Cols, Rows> operator-() const
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
        for (const auto& pos : dmath::sbounds2{ { OtherCols, Rows } })
            for (std::size_t i = 0; i < Cols; i++)
                result[pos] += lhs(i, pos.y()) * rhs(pos.x(), i);
        return result;
    }

    template <std::size_t OtherCols>
    friend inline constexpr std::optional<Matrix<T, Cols, Rows>> operator/(Matrix<T, Cols, Rows> lhs, const Matrix<T, OtherCols, Cols>& rhs)
    {
        if (auto inv = rhs.inverse())
            return lhs * *inv;
        return std::nullopt;
    }

    friend inline constexpr Matrix<T, Cols, Rows>& operator*=(Matrix<T, Cols, Rows>& matrix, T scalar)
    {
        return assignment(matrix, scalar, [](Vector<T, Rows>& a, Vector<T, Rows> b) { a *= b; });
    }

    friend inline constexpr Matrix<T, Cols, Rows>& operator/=(Matrix<T, Cols, Rows>& matrix, T scalar)
    {
        return assignment(matrix, scalar, [](Vector<T, Rows>& a, Vector<T, Rows> b) { a /= b; });
    }

    friend inline constexpr Matrix<T, Cols, Rows> operator*(Matrix<T, Cols, Rows> matrix, T scalar)
    {
        return matrix *= scalar;
    }

    friend inline constexpr Matrix<T, Cols, Rows> operator*(T scalar, Matrix<T, Cols, Rows> matrix)
    {
        return matrix *= scalar;
    }

    friend inline constexpr Matrix<T, Cols, Rows> operator/(Matrix<T, Cols, Rows> matrix, T scalar)
    {
        return matrix /= scalar;
    }

    friend inline constexpr std::optional<Matrix<T, Cols, Rows>> operator/(T scalar, const Matrix<T, Cols, Rows>& matrix)
    {
        if (auto inv = matrix.inverse())
            return scalar * *inv;
        return std::nullopt;
    }

    friend inline constexpr Vector<T, Rows> operator*(Matrix<T, Cols, Rows> matrix, Vector<T, Cols> vector)
    {
        return matrix * Matrix<T, 1, Cols>(vector);
    }

    friend inline constexpr Vector<T, Cols> operator*(Vector<T, Rows> vector, Matrix<T, Cols, Rows> matrix)
    {
        return matrix.transpose() * vector;
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

    inline constexpr typename Base::iterator begin()
    {
        return Base::begin();
    }

    inline constexpr typename Base::iterator end()
    {
        return Base::end();
    }

    inline constexpr typename Base::const_iterator begin() const
    {
        return Base::begin();
    }

    inline constexpr typename Base::const_iterator end() const
    {
        return Base::end();
    }

    inline constexpr typename Base::const_iterator cbegin() const
    {
        return Base::cbegin();
    }

    inline constexpr typename Base::const_iterator cend() const
    {
        return Base::cend();
    }

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

#define DMATH_MATRIX_DEFINE(mtype, vtype) \
template <std::size_t Cols, std::size_t Rows = Cols> \
using mtype = Matrix<vtype, Cols, Rows>; \
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

DMATH_MATRIX_DEFINE(mat, float)
DMATH_MATRIX_DEFINE(dmat, double)
DMATH_MATRIX_DEFINE(imat, int)
DMATH_MATRIX_DEFINE(umat, unsigned)
DMATH_MATRIX_DEFINE(smat, std::size_t)

#undef DMATH_MATRIX_DEFINE

}
