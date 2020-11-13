#pragma once

#include "utils.h"

#include "bounds.h"
#include "vector.h"

namespace dang::math {

/// <summary>A generic, column-major matrix of any dimensions.</summary>
template <typename T, std::size_t Cols, std::size_t Rows = Cols>
struct Matrix : protected std::array<Vector<T, Rows>, Cols> {
    using Base = std::array<Vector<T, Rows>, Cols>;

    /// <summary>Initializes the matrix with zero.</summary>
    constexpr Matrix()
        : Base()
    {}

    /// <summary>Initializes the matrix from a std::array of columns.</summary>
    constexpr Matrix(const Base& columns)
        : Base(columns)
    {}

    /// <summary>Initializes the whole matrix with the same, given value.</summary>
    constexpr Matrix(T value)
        : Base()
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = value;
    }

    /// <summary>Initializes the matrix from a C-style array of columns.</summary>
    constexpr Matrix(const Vector<T, Rows> (&columns)[Cols])
        : Base()
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = columns[i];
    }

    /// <summary>Initializes a single-column matrix with the given values.</summary>
    constexpr Matrix(Vector<T, Rows> col)
        : Base({col})
    {
        static_assert(Cols == 1, "only mat1xN can be constructed from vector");
    }

    using Base::operator[];

    /// <summary>Returns the identity matrix.</summary>
    /// <remarks>For non-square matrices the rest is filled with zeros.</remarks>
    static constexpr Matrix identity()
    {
        Matrix result;
        for (std::size_t i = 0; i < (Cols < Rows ? Cols : Rows); i++)
            result(i, i) = T(1);
        return result;
    }

    /// <summary>Allows for implicit conversion from single-column matrices to vectors.</summary>
    constexpr operator Vector<T, Rows>() const
    {
        static_assert(Cols == 1, "only mat1xN can be converted to vector");
        return (*this)[0];
    }

    /// <summary>Allows for implicit conversion from single-value matrices to their respective value type.</summary>
    constexpr operator T() const
    {
        static_assert(Cols == 1 && Rows == 1, "only mat1x1 can be converted to value type");
        return (*this)(0, 0);
    }

    /// <summary>Returns a sub matrix with the given offset and size.</summary>
    template <std::size_t StartCol, std::size_t StartRow, std::size_t ColCount, std::size_t RowCount>
    constexpr Matrix<T, ColCount, RowCount> subMatrix() const
    {
        Matrix<T, ColCount, RowCount> result;
        for (const auto& [col, row] : sbounds2(svec2(ColCount, RowCount)))
            result(col, row) = (*this)(StartCol + col, StartRow + row);
        return result;
    }

    /// <summary>Sets a sub matrix at the given offset and size.</summary>
    template <std::size_t StartCol, std::size_t StartRow, std::size_t ColCount, std::size_t RowCount>
    constexpr void setSubMatrix(Matrix<T, ColCount, RowCount> matrix)
    {
        for (const auto& [col, row] : sbounds2(svec2(ColCount, RowCount)))
            (*this)(StartCol + col, StartRow + row) = matrix(col, row);
    }

    /// <summary>Returns a reference to the value at the given position. (x = col, y = row)</summary>
    constexpr T& operator[](const dmath::svec2& pos) { return (*this)(pos.x(), pos.y()); }

    /// <summary>Returns a const reference to the value at the given position. (x = col, y = row)</summary>
    constexpr const T& operator[](const dmath::svec2& pos) const { return (*this)(pos.x(), pos.y()); }

    /// <summary>Returns a reference to the value at the given column/row.</summary>
    constexpr T& operator()(std::size_t col, std::size_t row) { return (*this)[col][row]; }

    /// <summary>Returns a const reference to the value at the given column/row.</summary>
    constexpr const T& operator()(std::size_t col, std::size_t row) const { return (*this)[col][row]; }

    /// <summary>Returns the transposed matrix.</summary>
    constexpr Matrix<T, Rows, Cols> transpose() const
    {
        Matrix<T, Rows, Cols> result;
        for (const auto& pos : dmath::sbounds2{{Cols, Rows}})
            result[pos.yx()] = (*this)[pos];
        return result;
    }

    /// <summary>Returns the minor at the given column/row.</summary>
    /// <remarks>A minor is exactly one column and one row smaller than the original, as the specified column and row are removed from the matrix.</remarks>
    constexpr Matrix<T, Cols - 1, Rows - 1> minor(std::size_t col, std::size_t row) const { return minor({col, row}); }

    /// <summary>Returns the minor at the given position. (x = col, y = row)</summary>
    /// <remarks>The minor is exactly one column and one row smaller than the original, as the specified column and row are removed from the matrix.</remarks>
    constexpr Matrix<T, Cols - 1, Rows - 1> minor(const dmath::svec2& pos) const
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

    /// <summary>Returns the cofactor at the given column/row.</summary>
    /// <remarks>The cofactor is the determinant of the minor at the specified column/row and negated, if column + row is odd.</remarks>
    constexpr T cofactor(std::size_t col, std::size_t row) const
    {
        static_assert(std::is_signed_v<T>, "mat::cofactor requires a signed value");
        return cofactor({col, row});
    }

    /// <summary>Returns the cofactor at the given position. (x = col, y = row)</summary>
    /// <remarks>The cofactor is the determinant of the minor at the specified position and negated, if x + y is odd.</remarks>
    constexpr T cofactor(const dmath::svec2& pos) const
    {
        static_assert(std::is_signed_v<T>, "mat::cofactor requires a signed value");
        const T factor = T(1) - ((pos.x() + pos.y()) & 1) * 2;
        return minor(pos).determinant() * factor;
    }

    /// <summary>Returns a new matrix, where each element is the cofactor at the given position.</summary>
    constexpr Matrix<T, Cols, Rows> cofactorMatrix() const
    {
        static_assert(std::is_signed_v<T>, "mat::cofactorMatrix requires a signed value");
        Matrix<T, Cols, Rows> result;
        for (const auto& pos : dmath::sbounds2{{Cols, Rows}})
            result[pos] = cofactor(pos);
        return result;
    }

    /// <summary>Return the adjugate of the matrix, which is simply the transposed cofactor-matrix.</summary>
    constexpr Matrix<T, Rows, Cols> adjugate() const
    {
        static_assert(std::is_signed_v<T>, "mat::adjugate requires a signed value");
        if constexpr (Rows == 1 && Cols == 1)
            return identity();
        else
            return cofactorMatrix().transpose();
    }

    /// <summary>Returns the inverse of the matrix.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Dim &lt;= 4: Cramer's rule</para>
    /// <para>Dim > 4: Blockwise inversion (recursive)</para>
    /// </remarks>
    constexpr std::optional<Matrix<T, Rows, Cols>> inverse() const
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

    /// <summary>Returns the determinant of the matrix.</summary>
    /// <remarks>Up to 3x3 is hard-coded, otherwise uses very costly recursion.</remarks>
    constexpr T determinant() const
    {
        static_assert(std::is_floating_point_v<T>, "mat::determinant requires a floating point type");
        constexpr std::size_t Dim = Cols < Rows ? Cols : Rows;
        if constexpr (Dim == 1) {
            return *this;
        }
        else if constexpr (Dim == 2) {
            return (*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0);
        }
        else if constexpr (Dim == 3) {
            return (*this)(0, 0) * (*this)(1, 1) * (*this)(2, 2) + (*this)(0, 1) * (*this)(1, 2) * (*this)(2, 0) +
                   (*this)(0, 2) * (*this)(1, 0) * (*this)(2, 1) - (*this)(2, 0) * (*this)(1, 1) * (*this)(0, 2) -
                   (*this)(2, 1) * (*this)(1, 2) * (*this)(0, 0) - (*this)(2, 2) * (*this)(1, 0) * (*this)(0, 1);
        }
        else {
            T result = T();
            for (std::size_t i = 0; i < Dim; i++)
                result += (*this)(i, 0) * cofactor(i, 0);
            return result;
        }
    }

    /// <summary>Returns true, if the matrix is solvable, when seen as a linear equation.</summary>
    /// <remarks>Simply returns true, if the determinant is not zero.</remarks>
    constexpr bool solvable() const { return determinant() != T(); }

    /// <summary>Solves a single column of the matrix, when seen as a linear equation.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 6: Inverse</para>
    /// <para>Unknowns &lt; 6: Column-swap and determinant. (Swaps performed in-place)</para>
    /// </remarks>
    constexpr std::optional<T> solveCol(std::size_t col)
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

    /// <summary>Solves a single column of the matrix, when seen as a linear equation.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 6: Inverse</para>
    /// <para>Unknowns &lt; 6: Column-swap and determinant. (Swaps not performed in-place)</para>
    /// </remarks>
    constexpr std::optional<T> solveCol(std::size_t col) const
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
            return swappedMatrix.determinant() / oldDeterminant;
        }
    }

    /// <summary>Solves a single column of the matrix, when seen as a linear equation in combination with the given vector.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 6: Inverse</para>
    /// <para>Unknowns &lt; 6: Column-swap and determinant. (Swaps performed in-place)</para>
    /// </remarks>
    constexpr std::optional<T> solveCol(std::size_t col, Vector<T, Cols> vector)
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

    /// <summary>Solves a single column of the matrix, when seen as a linear equation in combination with the given vector.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 6: Inverse</para>
    /// <para>Unknowns &lt; 6: Column-swap and determinant. (Swaps not performed in-place)</para>
    /// </remarks>
    constexpr std::optional<T> solveCol(std::size_t col, Vector<T, Cols> vector) const
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

    /// <summary>Solves the matrix, when seen as a linear equation.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 5: Inverse</para>
    /// <para>Unknowns &lt; 5: Column-swap and determinant. (Swaps performed in-place)</para>
    /// </remarks>
    constexpr std::optional<Vector<T, Rows>> solve()
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

    /// <summary>Solves the matrix, when seen as a linear equation.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 5: Inverse</para>
    /// <para>Unknowns &lt; 5: Column-swap and determinant. (Swaps not performed in-place)</para>
    /// </remarks>
    constexpr std::optional<Vector<T, Rows>> solve() const
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

    /// <summary>Solves the matrix, when seen as a linear equation in combination with the given vector.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 5: Inverse</para>
    /// <para>Unknowns &lt; 5: Column-swap and determinant. (Swaps performed in-place)</para>
    /// </remarks>
    constexpr std::optional<Vector<T, Cols>> solve(Vector<T, Cols> vector)
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

    /// <summary>Solves the matrix, when seen as a linear equation in combination with the given vector.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 5: Inverse</para>
    /// <para>Unknowns &lt; 5: Column-swap and determinant. (Swaps not performed in-place)</para>
    /// </remarks>
    constexpr std::optional<Vector<T, Cols>> solve(Vector<T, Cols> vector) const
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

    /// <summary>Returns the matrix itself.</summary>
    constexpr Matrix<T, Cols, Rows> operator+() const { return *this; }

    /// <summary>Returns a component-wise negation of the matrix.</summary>
    constexpr Matrix<T, Cols, Rows> operator-() const
    {
        return unary([](Vector<T, Rows> a) { return -a; });
    }

#define DMATH_MATRIX_OPERATION(op)                                                                                     \
    friend constexpr Matrix<T, Cols, Rows> operator op(Matrix<T, Cols, Rows> lhs, const Matrix<T, Cols, Rows>& rhs)    \
    {                                                                                                                  \
        return lhs op## = rhs;                                                                                         \
    }                                                                                                                  \
    friend constexpr Matrix<T, Cols, Rows>& operator op##=(Matrix<T, Cols, Rows>& lhs,                                 \
                                                           const Matrix<T, Cols, Rows>& rhs)                           \
    {                                                                                                                  \
        return assignment(lhs, rhs, [](Vector<T, Rows>& a, Vector<T, Rows> b) { a op## = b; });                        \
    }

    /// <summary>Performs component-wise addition of two matrices.</summary>
    DMATH_MATRIX_OPERATION(+);
    /// <summary>Performs component-wise subtraction of two matrices.</summary>
    DMATH_MATRIX_OPERATION(-);

#undef DMATH_MATRIX_OPERATION

    /// <summary>Performs a matrix-multiplication between the two matrices.</summary>
    template <std::size_t OtherCols>
    friend constexpr Matrix<T, OtherCols, Rows> operator*(const Matrix<T, Cols, Rows>& lhs,
                                                          const Matrix<T, OtherCols, Cols>& rhs)
    {
        Matrix<T, OtherCols, Rows> result;
        for (const auto& pos : dmath::sbounds2{{OtherCols, Rows}})
            for (std::size_t i = 0; i < Cols; i++)
                result[pos] += lhs(i, pos.y()) * rhs(pos.x(), i);
        return result;
    }

    /// <summary>Performs a matrix-multiplication with the inverse of rhs.</summary>
    template <std::size_t OtherCols>
    friend constexpr std::optional<Matrix<T, Cols, Rows>> operator/(Matrix<T, Cols, Rows> lhs,
                                                                    const Matrix<T, OtherCols, Cols>& rhs)
    {
        if (auto inv = rhs.inverse())
            return lhs * *inv;
        return std::nullopt;
    }

    /// <summary>Performs a component-wise multiplication with the given scalar.</summary>
    friend constexpr Matrix<T, Cols, Rows>& operator*=(Matrix<T, Cols, Rows>& matrix, T scalar)
    {
        return assignment(matrix, scalar, [](Vector<T, Rows>& a, Vector<T, Rows> b) { a *= b; });
    }

    /// <summary>Performs a component-wise division with the given scalar.</summary>
    friend constexpr Matrix<T, Cols, Rows>& operator/=(Matrix<T, Cols, Rows>& matrix, T scalar)
    {
        return assignment(matrix, scalar, [](Vector<T, Rows>& a, Vector<T, Rows> b) { a /= b; });
    }

    /// <summary>Performs a component-wise multiplication with the given scalar.</summary>
    friend constexpr Matrix<T, Cols, Rows> operator*(Matrix<T, Cols, Rows> matrix, T scalar)
    {
        return matrix *= scalar;
    }

    /// <summary>Performs a component-wise multiplication with the given scalar.</summary>
    friend constexpr Matrix<T, Cols, Rows> operator*(T scalar, Matrix<T, Cols, Rows> matrix)
    {
        return matrix *= scalar;
    }

    /// <summary>Performs a component-wise division with the given scalar.</summary>
    friend constexpr Matrix<T, Cols, Rows> operator/(Matrix<T, Cols, Rows> matrix, T scalar)
    {
        return matrix /= scalar;
    }

    /// <summary>Performs a component-wise multiplication with the inverse of the matrix.</summary>
    friend constexpr std::optional<Matrix<T, Cols, Rows>> operator/(T scalar, const Matrix<T, Cols, Rows>& matrix)
    {
        if (auto inv = matrix.inverse())
            return scalar * *inv;
        return std::nullopt;
    }

    /// <summary>Performs a matrix-multiplication between the matrix and the given vector, seen as a single-column matrix.</summary>
    friend constexpr Vector<T, Rows> operator*(Matrix<T, Cols, Rows> matrix, Vector<T, Cols> vector)
    {
        return matrix * Matrix<T, 1, Cols>(vector);
    }

    /// <summary>Performs a matrix-multiplication between the transpose of the matrix and the given vector, seen as a single-column matrix.</summary>
    friend constexpr Vector<T, Cols> operator*(Vector<T, Rows> vector, Matrix<T, Cols, Rows> matrix)
    {
        return matrix.transpose() * vector;
    }

#define DMATH_MATRIX_COMPARE(merge, op)                                                                                \
    friend constexpr bool operator op(const Matrix<T, Cols, Rows>& lhs, const Matrix<T, Cols, Rows>& rhs)              \
    {                                                                                                                  \
        return lhs.merge(rhs, [](Vector<T, Rows> a, Vector<T, Rows> b) { return a op b; });                            \
    }

    /// <summary>Returns true, if all elements are identical.</summary>
    DMATH_MATRIX_COMPARE(all, ==);
    /// <summary>Returns true, if any elements differ.</summary>
    DMATH_MATRIX_COMPARE(any, !=);
    /// <summary>Returns true, if all elements of lhs are smaller than rhs.</summary>
    DMATH_MATRIX_COMPARE(all, <);
    /// <summary>Returns true, if all elements of lhs are smaller than or equal to rhs.</summary>
    DMATH_MATRIX_COMPARE(all, <=);
    /// <summary>Returns true, if all elements of lhs are greater than rhs.</summary>
    DMATH_MATRIX_COMPARE(all, >);
    /// <summary>Returns true, if all elements of lhs are greater than equal to rhs.</summary>
    DMATH_MATRIX_COMPARE(all, >=);

#undef DMATH_MATRIX_COMPARE

    using Base::begin;
    using Base::end;

    using Base::cbegin;
    using Base::cend;

private:
    template <typename Op>
    static constexpr Matrix<T, Cols, Rows>& assignment(Matrix<T, Cols, Rows>& lhs,
                                                       const Matrix<T, Cols, Rows>& rhs,
                                                       const Op& op)
    {
        for (std::size_t i = 0; i < Cols; i++)
            op(lhs[i], rhs[i]);
        return lhs;
    }

    template <typename Op>
    constexpr Matrix<T, Cols, Rows> binary(const Matrix<T, Cols, Rows>& other, const Op& op) const
    {
        Matrix<T, Cols, Rows> result;
        for (std::size_t i = 0; i < Cols; i++)
            result[i] = op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    constexpr Matrix<T, Cols, Rows> unary(const Op& op) const
    {
        Matrix<T, Cols, Rows> result;
        for (std::size_t i = 0; i < Cols; i++)
            result[i] = op((*this)[i]);
        return result;
    }

    template <typename Op>
    constexpr bool all(const Matrix<T, Cols, Rows>& other, const Op& op) const
    {
        bool result = true;
        for (std::size_t i = 0; i < Cols; i++)
            result = result && op((*this)[i], other[i]);
        return result;
    }

    template <typename Op>
    constexpr bool any(const Matrix<T, Cols, Rows>& other, const Op& op) const
    {
        bool result = false;
        for (std::size_t i = 0; i < Cols; i++)
            result = result || op((*this)[i], other[i]);
        return result;
    }
};

#define DMATH_MATRIX_DEFINE(name, type)                                                                                \
    template <std::size_t Cols, std::size_t Rows = Cols>                                                               \
    using name = dang::math::Matrix<type, Cols, Rows>;                                                                 \
    using name##1x1 = name<1, 1>;                                                                                      \
    using name##1x2 = name<1, 2>;                                                                                      \
    using name##1x3 = name<1, 3>;                                                                                      \
    using name##1x4 = name<1, 4>;                                                                                      \
    using name##2x1 = name<2, 1>;                                                                                      \
    using name##2x2 = name<2, 2>;                                                                                      \
    using name##2x3 = name<2, 3>;                                                                                      \
    using name##2x4 = name<2, 4>;                                                                                      \
    using name##3x1 = name<3, 1>;                                                                                      \
    using name##3x2 = name<3, 2>;                                                                                      \
    using name##3x3 = name<3, 3>;                                                                                      \
    using name##3x4 = name<3, 4>;                                                                                      \
    using name##4x1 = name<4, 1>;                                                                                      \
    using name##4x2 = name<4, 2>;                                                                                      \
    using name##4x3 = name<4, 3>;                                                                                      \
    using name##4x4 = name<4, 4>;                                                                                      \
    using name##1 = name##1x1;                                                                                         \
    using name##2 = name##2x2;                                                                                         \
    using name##3 = name##3x3;                                                                                         \
    using name##4 = name##4x4;

DMATH_MATRIX_DEFINE(mat, float)
DMATH_MATRIX_DEFINE(dmat, double)
DMATH_MATRIX_DEFINE(imat, int)
DMATH_MATRIX_DEFINE(umat, unsigned)
DMATH_MATRIX_DEFINE(smat, std::size_t)

} // namespace dang::math
