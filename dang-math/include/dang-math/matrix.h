#pragma once

#include "utils.h"

#include "bounds.h"
#include "vector.h"

namespace dang::math {

/// <summary>A generic, column-major matrix of any dimensions.</summary>
template <typename T, std::size_t Cols, std::size_t Rows = Cols>
struct Matrix : std::array<Vector<T, Rows>, Cols> {
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
        static_assert(Cols == 1);
    }

    /// <summary>Returns the identity matrix, optionally multiplied with a scalar.</summary>
    /// <remarks>For non-square matrices the rest is filled with zeros.</remarks>
    static constexpr auto identity(T value = T{1})
    {
        Matrix result;
        for (std::size_t i = 0; i < (Cols < Rows ? Cols : Rows); i++)
            result(i, i) = value;
        return result;
    }

    /// <summary>Allows for implicit conversion from single-column matrices to vectors.</summary>
    constexpr operator Vector<T, Rows>() const
    {
        static_assert(Cols == 1);
        return (*this)[0];
    }

    /// <summary>Allows for implicit conversion from single-value matrices to their respective value type.</summary>
    constexpr operator T() const
    {
        static_assert(Cols == 1 && Rows == 1);
        return (*this)(0, 0);
    }

    /// <summary>Returns a sub matrix with the given offset and size.</summary>
    template <std::size_t StartCol, std::size_t StartRow, std::size_t ColCount, std::size_t RowCount>
    constexpr auto subMatrix() const
    {
        Matrix<T, ColCount, RowCount> result;
        sbounds2 bounds{{ColCount, RowCount}};
        for (const auto& [col, row] : bounds)
            result(col, row) = (*this)(StartCol + col, StartRow + row);
        return result;
    }

    /// <summary>Sets a sub matrix at the given offset and size.</summary>
    template <std::size_t StartCol, std::size_t StartRow, std::size_t ColCount, std::size_t RowCount>
    constexpr void setSubMatrix(Matrix<T, ColCount, RowCount> matrix)
    {
        sbounds2 bounds{{ColCount, RowCount}};
        for (const auto& [col, row] : bounds)
            (*this)(StartCol + col, StartRow + row) = matrix(col, row);
    }

    using Base::operator[];

    /// <summary>Returns a reference to the value at the given position. (x = col, y = row)</summary>
    constexpr auto& operator[](const dmath::svec2& pos) { return (*this)(pos.x(), pos.y()); }

    /// <summary>Returns a const reference to the value at the given position. (x = col, y = row)</summary>
    constexpr const auto& operator[](const dmath::svec2& pos) const { return (*this)(pos.x(), pos.y()); }

    /// <summary>Returns a reference to the value at the given column/row.</summary>
    constexpr auto& operator()(std::size_t col, std::size_t row) { return (*this)[col][row]; }

    /// <summary>Returns a const reference to the value at the given column/row.</summary>
    constexpr const auto& operator()(std::size_t col, std::size_t row) const { return (*this)[col][row]; }

    /// <summary>Returns the transposed matrix.</summary>
    constexpr auto transpose() const
    {
        Matrix<T, Rows, Cols> result;
        dmath::sbounds2 bounds{{Cols, Rows}};
        for (auto [x, y] : bounds)
            result(y, x) = (*this)(x, y);
        return result;
    }

    /// <summary>Returns the minor at the given column/row.</summary>
    /// <remarks>A minor is exactly one column and one row smaller than the original, as the specified column and row are removed from the matrix.</remarks>
    constexpr auto minor(std::size_t col, std::size_t row) const
    {
        static_assert(Cols > 0 && Rows > 0);
        return minor({col, row});
    }

    /// <summary>Returns the minor at the given position. (x = col, y = row)</summary>
    /// <remarks>The minor is exactly one column and one row smaller than the original, as the specified column and row are removed from the matrix.</remarks>
    constexpr auto minor(const dmath::svec2& pos) const
    {
        static_assert(Cols > 0 && Rows > 0);
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
    constexpr auto cofactor(std::size_t col, std::size_t row) const
    {
        static_assert(Cols > 0 && Rows > 0);
        return cofactor({col, row});
    }

    /// <summary>Returns the cofactor at the given position. (x = col, y = row)</summary>
    /// <remarks>The cofactor is the determinant of the minor at the specified position and negated, if x + y is odd.</remarks>
    constexpr auto cofactor(const dmath::svec2& pos) const
    {
        static_assert(Cols > 0 && Rows > 0);
        const T factor = T{1} - ((pos.x() + pos.y()) & 1) * 2;
        return minor(pos).determinant() * factor;
    }

    /// <summary>Returns a new matrix, where each element is the cofactor at the given position.</summary>
    constexpr auto cofactorMatrix() const
    {
        Matrix result;
        if constexpr (Cols > 0 && Rows > 0) {
            dmath::sbounds2 bounds{{Cols, Rows}};
            for (const auto& pos : bounds)
                result[pos] = cofactor(pos);
        }
        return result;
    }

    /// <summary>Return the adjugate of the matrix, which is simply the transposed cofactor-matrix.</summary>
    constexpr auto adjugate() const
    {
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
    constexpr std::optional<Matrix> inverse() const
    {
        static_assert(Cols == Rows);

        constexpr std::size_t Dim = Cols;
        constexpr std::size_t DimHalf1 = Dim / 2 + Dim % 2;
        constexpr std::size_t DimHalf2 = Dim / 2;

        if constexpr (Dim <= 4) {
            T det = determinant();
            if (det == T{})
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

            Matrix result;
            result.setSubMatrix<0, 0, DimHalf1, DimHalf1>(e);
            result.setSubMatrix<DimHalf1, 0, DimHalf2, DimHalf1>(f);
            result.setSubMatrix<0, DimHalf1, DimHalf1, DimHalf2>(g);
            result.setSubMatrix<DimHalf1, DimHalf1, DimHalf2, DimHalf2>(h);
            return result;
        }
    }

    /// <summary>Returns the determinant of the matrix.</summary>
    /// <remarks>Up to 3x3 is hard-coded, otherwise uses very costly recursion.</remarks>
    constexpr auto determinant() const
    {
        constexpr std::size_t Dim = Cols < Rows ? Cols : Rows;
        if constexpr (Dim == 1) {
            return (*this)(0, 0);
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
            T result{};
            if constexpr (Dim > 0) {
                for (std::size_t i = 0; i < Dim; i++)
                    result += (*this)(i, 0) * cofactor(i, 0);
            }
            return result;
        }
    }

    /// <summary>Returns true, if the matrix is solvable, when seen as a linear equation.</summary>
    /// <remarks>Simply returns true, if the determinant is not zero.</remarks>
    constexpr auto solvable() const { return determinant() != T{}; }

    /// <summary>Solves a single column of the matrix, when seen as a linear equation.</summary>
    /// <remarks>
    /// <para>Algorithms used:</para>
    /// <para>Unknowns >= 6: Inverse</para>
    /// <para>Unknowns &lt; 6: Column-swap and determinant. (Swaps performed in-place)</para>
    /// </remarks>
    constexpr std::optional<T> solveCol(std::size_t col)
    {
        static_assert(Cols == Rows + 1);

        if constexpr (Rows >= 6) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return (*inv * (*this)[Rows])[col];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            auto tmp = (*this)[col];
            (*this)[col] = (*this)[Rows];
            (*this)[Rows] = tmp;

            T result = determinant() / old_determinant;

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
        static_assert(Cols == Rows + 1);

        if constexpr (Rows >= 6) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return (*inv * (*this)[Rows])[col];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            auto swapped_matrix = *this;
            swapped_matrix[col] = (*this)[Rows];
            return swapped_matrix.determinant() / old_determinant;
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
        static_assert(Cols == Rows);

        if constexpr (Rows >= 6) {
            if (auto inv = inverse())
                return (*inv * vector)[col];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            auto tmp = (*this)[col];
            (*this)[col] = vector;
            vector = tmp;

            T result = determinant() / old_determinant;

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
        static_assert(Cols == Rows);

        if constexpr (Rows >= 6) {
            if (auto inv = inverse())
                return (*inv * vector)[col];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            auto swapped_matrix = *this;
            swapped_matrix[col] = vector;
            return swapped_matrix.determinant() / old_determinant;
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
        static_assert(Cols == Rows + 1);

        if constexpr (Rows >= 5) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return *inv * (*this)[Rows];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, Rows> result;

            for (std::size_t col = 0; col < Rows; col++) {
                auto tmp = (*this)[col];
                (*this)[col] = (*this)[Rows];
                (*this)[Rows] = tmp;

                result[col] = determinant() / old_determinant;

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
        static_assert(Cols == Rows + 1);

        if constexpr (Rows >= 5) {
            if (auto inv = subMatrix<0, 0, Rows, Rows>().inverse())
                return *inv * (*this)[Rows];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, Rows> result;
            auto swapped_matrix = *this;

            for (std::size_t col = 0; col < Rows; col++) {
                auto tmp = swapped_matrix[col];
                swapped_matrix[col] = swapped_matrix[Rows];
                swapped_matrix[Rows] = tmp;

                result[col] = swapped_matrix.determinant() / old_determinant;

                swapped_matrix[Rows] = swapped_matrix[col];
                swapped_matrix[col] = tmp;
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
        static_assert(Cols == Rows);

        if constexpr (Rows >= 5) {
            if (auto inv = inverse())
                return *inv * vector;
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, Cols> result;

            for (std::size_t col = 0; col < Cols; col++) {
                auto tmp = (*this)[col];
                (*this)[col] = vector;
                vector = tmp;

                result[col] = determinant() / old_determinant;

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
        static_assert(Cols == Rows);

        if constexpr (Rows >= 5) {
            if (auto inv = inverse())
                return *inv * vector;
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, Cols> result;
            auto swapped_matrix = *this;

            for (std::size_t col = 0; col < Cols; col++) {
                auto tmp = swapped_matrix[col];
                swapped_matrix[col] = vector;
                vector = tmp;

                result[col] = swapped_matrix.determinant() / old_determinant;

                vector = swapped_matrix[col];
                swapped_matrix[col] = tmp;
            }

            return result;
        }
    }

    /// <summary>Returns the matrix itself.</summary>
    constexpr auto operator+() const { return *this; }

    /// <summary>Returns a component-wise negation of the matrix.</summary>
    constexpr auto operator-() const
    {
        static_assert(std::is_signed_v<T>);
        return variadicOp(std::negate<>{});
    }

    /// <summary>Performs a component-wise addition.</summary>
    friend constexpr auto operator+(const Matrix& lhs, const Matrix& rhs) { return lhs.variadicOp(std::plus<>{}, rhs); }

    /// <summary>Performs a component-wise addition.</summary>
    constexpr auto& operator+=(const Matrix& other) { return assignmentOp(std::plus<>{}, other); }

    /// <summary>Performs a component-wise subtraction.</summary>
    friend constexpr auto operator-(const Matrix& lhs, const Matrix& rhs)
    {
        return lhs.variadicOp(std::minus<>{}, rhs);
    }

    /// <summary>Performs a component-wise subtraction.</summary>
    constexpr auto& operator-=(const Matrix& other) { return assignmentOp(std::minus<>{}, other); }

    /// <summary>Performs a component-wise multiplication.</summary>
    constexpr auto compMul(const Matrix& other) const { return variadicOp(std::multiplies<>{}, other); }

    /// <summary>Performs a component-wise division.</summary>
    constexpr auto compDiv(const Matrix& other) const { return variadicOp(std::divides<>{}, other); }

    /// <summary>Performs a component-wise multiplication with the given scalar.</summary>
    constexpr auto operator*(T scalar) const { return variadicOp(std::multiplies<>{}, Matrix{scalar}); }

    /// <summary>Performs a component-wise multiplication with the given scalar.</summary>
    friend constexpr auto operator*(T scalar, const Matrix& matrix) { return matrix * scalar; }

    /// <summary>Performs a component-wise multiplication with the given scalar.</summary>
    constexpr auto& operator*=(T scalar) { return assignmentOp(std::multiplies<>{}, Matrix{scalar}); }

    /// <summary>Performs a component-wise division with the given scalar.</summary>
    constexpr auto operator/(T scalar) const { return variadicOp(std::divides<>{}, Matrix{scalar}); }

    /// <summary>Performs a component-wise multiplication with the inverse of the matrix.</summary>
    friend constexpr std::optional<Matrix> operator/(T scalar, const Matrix& matrix)
    {
        if (auto inv = matrix.inverse())
            return scalar * *inv;
        return std::nullopt;
    }

    /// <summary>Performs a component-wise division with the given scalar.</summary>
    constexpr auto& operator/=(T scalar) { return assignmentOp(std::divides<>{}, Matrix{scalar}); }

    /// <summary>Performs a matrix-multiplication between the two matrices.</summary>
    template <std::size_t OtherCols>
    friend constexpr auto operator*(const Matrix& lhs, const Matrix<T, OtherCols, Cols>& rhs)
    {
        Matrix<T, OtherCols, Rows> result;
        sbounds2 bounds{{OtherCols, Rows}};
        for (const auto& pos : bounds)
            for (std::size_t i = 0; i < Cols; i++)
                result[pos] += lhs(i, pos.y()) * rhs(pos.x(), i);
        return result;
    }

    /// <summary>Performs a matrix-multiplication with the inverse of rhs.</summary>
    friend constexpr std::optional<Matrix> operator/(const Matrix& lhs, const Matrix<T, Cols, Cols>& rhs)
    {
        if (auto inv = rhs.inverse())
            return lhs * *inv;
        return std::nullopt;
    }

    /// <summary>Performs a matrix-multiplication between the matrix and the given vector, seen as a single-column matrix.</summary>
    friend constexpr Vector<T, Rows> operator*(const Matrix& matrix, const Vector<T, Cols>& vector)
    {
        return matrix * Matrix<T, 1, Cols>{vector};
    }

    /// <summary>Performs a matrix-multiplication between the transpose of the matrix and the given vector, seen as a single-column matrix.</summary>
    friend constexpr auto operator*(const Vector<T, Rows>& vector, const Matrix& matrix)
    {
        return matrix.transpose() * vector;
    }

    /// <summary>Performs a matrix-multiplication between the inverse of the matrix and the given vector, seen as a single-column matrix.</summary>
    friend constexpr std::optional<Vector<T, Rows>> operator/(const Vector<T, Rows>& vector, const Matrix& matrix)
    {
        static_assert(Cols == Rows);
        if (auto inv = matrix.inverse())
            return vector * *inv;
        return std::nullopt;
    }

    /// <summary>Performs an operation on each component using an arbitrary number of other vectors.</summary>
    template <typename TOperation, typename... TMatrices>
    constexpr auto variadicOp(TOperation operation, const TMatrices&... matrices) const
    {
        Matrix<T, Cols, Rows> result;
        for (std::size_t i = 0; i < Cols; i++)
            result[i] = operation((*this)[i], matrices[i]...);
        return result;
    }

    /// <summary>Performs an operation with another vector and assigns the result to itself.</summary>
    template <typename TOperation>
    constexpr auto& assignmentOp(TOperation operation, const Matrix& other)
    {
        for (std::size_t i = 0; i < Cols; i++)
            (*this)[i] = operation((*this)[i], other[i]);
        return *this;
    }

    /// <summary>Returns a multiline string representing the matrix.</summary>
    auto format() const { return (std::stringstream() << *this).str(); }

    /// <summary>Appends a string representation of the vector in the form [x, y, z] to the stream.</summary>
    friend auto& operator<<(std::ostream& stream, const Matrix& matrix)
    {
        for (std::size_t row = 0; row < Rows; row++) {
            std::cout << '[';
            if constexpr (Cols > 0)
                std::cout << matrix(0, row);
            for (std::size_t col = 1; col < Cols; col++)
                std::cout << ", " << matrix(col, row);
            std::cout << "]\n";
        }
        return stream;
    }
};

template <std::size_t Cols, std::size_t Rows = Cols>
using mat = Matrix<float, Cols, Rows>;
using mat2 = mat<2, 2>;
using mat2x3 = mat<2, 3>;
using mat2x4 = mat<2, 4>;
using mat3x2 = mat<3, 2>;
using mat3 = mat<3, 3>;
using mat3x4 = mat<3, 4>;
using mat4x2 = mat<4, 2>;
using mat4x3 = mat<4, 3>;
using mat4 = mat<4, 4>;

template <std::size_t Cols, std::size_t Rows = Cols>
using dmat = Matrix<double, Cols, Rows>;
using dmat2 = dmat<2, 2>;
using dmat2x3 = dmat<2, 3>;
using dmat2x4 = dmat<2, 4>;
using dmat3x2 = dmat<3, 2>;
using dmat3 = dmat<3, 3>;
using dmat3x4 = dmat<3, 4>;
using dmat4x2 = dmat<4, 2>;
using dmat4x3 = dmat<4, 3>;
using dmat4 = dmat<4, 4>;

} // namespace dang::math
