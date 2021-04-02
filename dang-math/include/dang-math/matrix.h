#pragma once

#include "dang-math/bounds.h"
#include "dang-math/global.h"
#include "dang-math/vector.h"

namespace dang::math {

/// @brief A generic, column-major matrix of any dimensions.
template <typename T, std::size_t v_cols, std::size_t v_rows = v_cols>
struct Matrix : std::array<Vector<T, v_rows>, v_cols> {
    using Base = std::array<Vector<T, v_rows>, v_cols>;

    /// @brief Initializes the matrix with zero.
    constexpr Matrix()
        : Base()
    {}

    /// @brief Initializes the matrix from a std::array of columns.
    constexpr Matrix(const Base& columns)
        : Base(columns)
    {}

    /// @brief Initializes the whole matrix with the same, given value.
    explicit constexpr Matrix(T value)
        : Base()
    {
        for (std::size_t i = 0; i < v_cols; i++)
            (*this)[i] = value;
    }

    /// @brief Initializes the matrix from a C-style array of columns.
    constexpr Matrix(const Vector<T, v_rows> (&columns)[v_cols])
        : Base()
    {
        for (std::size_t i = 0; i < v_cols; i++)
            (*this)[i] = columns[i];
    }

    /// @brief Initializes a single-column matrix with the given vector.
    static constexpr Matrix fromVector(const Vector<T, v_rows>& col)
    {
        static_assert(v_cols == 1);
        return {{col}};
    }

    /// @brief Initializes a two-column matrix from low and high of the given bounds.
    static constexpr Matrix fromBounds(const Bounds<T, v_rows>& bounds)
    {
        static_assert(v_cols == 2);
        return {{bounds.low, bounds.high}};
    }

    /// @brief Returns the identity matrix, optionally multiplied with a scalar.
    /// @remark For non-square matrices the rest is filled with zeros.
    static constexpr auto identity(T value = T{1})
    {
        Matrix result;
        for (std::size_t i = 0; i < (v_cols < v_rows ? v_cols : v_rows); i++)
            result(i, i) = value;
        return result;
    }

    /// @brief Allows for conversion from single-value matrices to their respective value type.
    explicit constexpr operator T() const
    {
        static_assert(v_cols == 1 && v_rows == 1);
        return (*this)(0, 0);
    }

    /// @brief Allows for conversion from single-column matrices to vectors.
    explicit constexpr operator Vector<T, v_rows>() const
    {
        static_assert(v_cols == 1);
        return (*this)[0];
    }

    /// @brief Allows for conversion from two-column matrices to bounds.
    explicit constexpr operator Bounds<T, v_rows>() const
    {
        static_assert(v_cols == 2);
        return {(*this)[0], (*this)[1]};
    }

    /// @brief Returns a sub matrix with the given offset and size.
    template <std::size_t v_start_col, std::size_t v_start_row, std::size_t v_col_count, std::size_t v_row_count>
    constexpr auto subMatrix() const
    {
        Matrix<T, v_col_count, v_row_count> result;
        sbounds2 bounds{{v_col_count, v_row_count}};
        for (const auto& [col, row] : bounds)
            result(col, row) = (*this)(v_start_col + col, v_start_row + row);
        return result;
    }

    /// @brief Sets a sub matrix at the given offset and size.
    template <std::size_t v_start_col, std::size_t v_start_row, std::size_t v_col_count, std::size_t v_row_count>
    constexpr void setSubMatrix(Matrix<T, v_col_count, v_row_count> matrix)
    {
        sbounds2 bounds{{v_col_count, v_row_count}};
        for (const auto& [col, row] : bounds)
            (*this)(v_start_col + col, v_start_row + row) = matrix(col, row);
    }

    using Base::operator[];

    /// @brief Returns a reference to the value at the given position. (x = col, y = row)
    constexpr auto& operator[](const svec2& pos) { return (*this)(pos.x(), pos.y()); }

    /// @brief Returns a const reference to the value at the given position. (x = col, y = row)
    constexpr const auto& operator[](const svec2& pos) const { return (*this)(pos.x(), pos.y()); }

    /// @brief Returns a reference to the value at the given column/row.
    constexpr auto& operator()(std::size_t col, std::size_t row) { return (*this)[col][row]; }

    /// @brief Returns a const reference to the value at the given column/row.
    constexpr const auto& operator()(std::size_t col, std::size_t row) const { return (*this)[col][row]; }

    /// @brief Returns the transposed matrix.
    constexpr auto transpose() const
    {
        Matrix<T, v_rows, v_cols> result;
        sbounds2 bounds{{v_cols, v_rows}};
        for (auto [x, y] : bounds)
            result(y, x) = (*this)(x, y);
        return result;
    }

    /// @brief Returns the minor at the given column/row.
    /// @remark A minor is exactly one column and one row smaller than the original, as the specified column and row are
    /// removed from the matrix.
    constexpr auto minor(std::size_t col, std::size_t row) const
    {
        static_assert(v_cols > 0 && v_rows > 0);
        return minor({col, row});
    }

    /// @brief Returns the minor at the given position. (x = col, y = row)
    /// @remark The minor is exactly one column and one row smaller than the original, as the specified column and row
    /// are removed from the matrix.
    constexpr auto minor(const svec2& pos) const
    {
        static_assert(v_cols > 0 && v_rows > 0);
        Matrix<T, v_cols - 1, v_rows - 1> result;
        std::size_t rcol = 0;
        for (std::size_t col = 0; col < v_cols; col++) {
            if (col == pos.x())
                continue;
            std::size_t rrow = 0;
            for (std::size_t row = 0; row < v_rows; row++) {
                if (row == pos.y())
                    continue;
                result(rcol, rrow) = (*this)(col, row);
                rrow++;
            }
            rcol++;
        }
        return result;
    }

    /// @brief Returns the cofactor at the given column/row.
    /// @remark The cofactor is the determinant of the minor at the specified column/row and negated, if column + row is
    /// odd.
    constexpr auto cofactor(std::size_t col, std::size_t row) const
    {
        static_assert(v_cols > 0 && v_rows > 0);
        return cofactor({col, row});
    }

    /// @brief Returns the cofactor at the given position. (x = col, y = row)
    /// @remark The cofactor is the determinant of the minor at the specified position and negated, if x + y is odd.
    constexpr auto cofactor(const svec2& pos) const
    {
        static_assert(v_cols > 0 && v_rows > 0);
        const T factor = T{1} - ((pos.x() + pos.y()) & 1) * 2;
        return minor(pos).determinant() * factor;
    }

    /// @brief Returns a new matrix, where each element is the cofactor at the given position.
    constexpr auto cofactorMatrix() const
    {
        Matrix result;
        if constexpr (v_cols > 0 && v_rows > 0) {
            sbounds2 bounds{{v_cols, v_rows}};
            for (const auto& pos : bounds)
                result[pos] = cofactor(pos);
        }
        return result;
    }

    /// @brief Return the adjugate of the matrix, which is simply the transposed cofactor-matrix.
    constexpr auto adjugate() const
    {
        if constexpr (v_rows == 1 && v_cols == 1)
            return identity();
        else
            return cofactorMatrix().transpose();
    }

    /// @brief Returns the inverse of the matrix.
    /// @remark
    /// Algorithms used:
    /// - Dim &lt;= 4: Cramer's rule
    /// - Dim > 4: Blockwise inversion (recursive)
    constexpr std::optional<Matrix> inverse() const
    {
        static_assert(v_cols == v_rows);

        constexpr std::size_t Dim = v_cols;
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

    /// @brief Returns the determinant of the matrix.
    /// @remark Up to 3x3 is hard-coded, otherwise uses very costly recursion.
    constexpr auto determinant() const
    {
        constexpr std::size_t Dim = v_cols < v_rows ? v_cols : v_rows;
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

    /// @brief Returns true, if the matrix is solvable, when seen as a linear equation.
    /// @remark Simply returns true, if the determinant is not zero.
    constexpr auto solvable() const { return determinant() != T{}; }

    /// @brief Solves a single column of the matrix, when seen as a linear equation.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 6: Inverse
    /// - Unknowns &lt; 6: Column-swap and determinant. (Swaps performed in-place)
    constexpr std::optional<T> solveCol(std::size_t col)
    {
        static_assert(v_cols == v_rows + 1);

        if constexpr (v_rows >= 6) {
            if (auto inv = subMatrix<0, 0, v_rows, v_rows>().inverse())
                return (*inv * (*this)[v_rows])[col];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            auto tmp = (*this)[col];
            (*this)[col] = (*this)[v_rows];
            (*this)[v_rows] = tmp;

            T result = determinant() / old_determinant;

            (*this)[v_rows] = (*this)[col];
            (*this)[col] = tmp;

            return result;
        }
    }

    /// @brief Solves a single column of the matrix, when seen as a linear equation.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 6: Inverse
    /// - Unknowns &lt; 6: Column-swap and determinant. (Swaps not performed in-place)
    constexpr std::optional<T> solveCol(std::size_t col) const
    {
        static_assert(v_cols == v_rows + 1);

        if constexpr (v_rows >= 6) {
            if (auto inv = subMatrix<0, 0, v_rows, v_rows>().inverse())
                return (*inv * (*this)[v_rows])[col];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            auto swapped_matrix = *this;
            swapped_matrix[col] = (*this)[v_rows];
            return swapped_matrix.determinant() / old_determinant;
        }
    }

    /// @brief Solves a single column of the matrix, when seen as a linear equation in combination with the given
    /// vector.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 6: Inverse
    /// - Unknowns &lt; 6: Column-swap and determinant. (Swaps performed in-place)
    constexpr std::optional<T> solveCol(std::size_t col, Vector<T, v_cols> vector)
    {
        static_assert(v_cols == v_rows);

        if constexpr (v_rows >= 6) {
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

    /// @brief Solves a single column of the matrix, when seen as a linear equation in combination with the given
    /// vector.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 6: Inverse
    /// - Unknowns &lt; 6: Column-swap and determinant. (Swaps not performed in-place)
    constexpr std::optional<T> solveCol(std::size_t col, Vector<T, v_cols> vector) const
    {
        static_assert(v_cols == v_rows);

        if constexpr (v_rows >= 6) {
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

    /// @brief Solves the matrix, when seen as a linear equation.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 5: Inverse
    /// - Unknowns &lt; 5: Column-swap and determinant. (Swaps performed in-place)
    constexpr std::optional<Vector<T, v_rows>> solve()
    {
        static_assert(v_cols == v_rows + 1);

        if constexpr (v_rows >= 5) {
            if (auto inv = subMatrix<0, 0, v_rows, v_rows>().inverse())
                return *inv * (*this)[v_rows];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, v_rows> result;

            for (std::size_t col = 0; col < v_rows; col++) {
                auto tmp = (*this)[col];
                (*this)[col] = (*this)[v_rows];
                (*this)[v_rows] = tmp;

                result[col] = determinant() / old_determinant;

                (*this)[v_rows] = (*this)[col];
                (*this)[col] = tmp;
            }

            return result;
        }
    }

    /// @brief Solves the matrix, when seen as a linear equation.
    /// @remark
    /// Algorithms used:
    /// Unknowns >= 5: Inverse
    /// Unknowns &lt; 5: Column-swap and determinant. (Swaps not performed in-place)
    constexpr std::optional<Vector<T, v_rows>> solve() const
    {
        static_assert(v_cols == v_rows + 1);

        if constexpr (v_rows >= 5) {
            if (auto inv = subMatrix<0, 0, v_rows, v_rows>().inverse())
                return *inv * (*this)[v_rows];
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, v_rows> result;
            auto swapped_matrix = *this;

            for (std::size_t col = 0; col < v_rows; col++) {
                auto tmp = swapped_matrix[col];
                swapped_matrix[col] = swapped_matrix[v_rows];
                swapped_matrix[v_rows] = tmp;

                result[col] = swapped_matrix.determinant() / old_determinant;

                swapped_matrix[v_rows] = swapped_matrix[col];
                swapped_matrix[col] = tmp;
            }

            return result;
        }
    }

    /// @brief Solves the matrix, when seen as a linear equation in combination with the given vector.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 5: Inverse
    /// - Unknowns &lt; 5: Column-swap and determinant. (Swaps performed in-place)
    constexpr std::optional<Vector<T, v_cols>> solve(Vector<T, v_cols> vector)
    {
        static_assert(v_cols == v_rows);

        if constexpr (v_rows >= 5) {
            if (auto inv = inverse())
                return *inv * vector;
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, v_cols> result;

            for (std::size_t col = 0; col < v_cols; col++) {
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

    /// @brief Solves the matrix, when seen as a linear equation in combination with the given vector.
    /// @remark
    /// Algorithms used:
    /// - Unknowns >= 5: Inverse
    /// - Unknowns &lt; 5: Column-swap and determinant. (Swaps not performed in-place)
    constexpr std::optional<Vector<T, v_cols>> solve(Vector<T, v_cols> vector) const
    {
        static_assert(v_cols == v_rows);

        if constexpr (v_rows >= 5) {
            if (auto inv = inverse())
                return *inv * vector;
            return std::nullopt;
        }
        else {
            T old_determinant = determinant();
            if (old_determinant == T{})
                return std::nullopt;

            Vector<T, v_cols> result;
            auto swapped_matrix = *this;

            for (std::size_t col = 0; col < v_cols; col++) {
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

    /// @brief Returns the matrix itself.
    constexpr auto operator+() const { return *this; }

    /// @brief Returns a component-wise negation of the matrix.
    constexpr auto operator-() const
    {
        static_assert(std::is_signed_v<T>);
        return variadicOp(std::negate{});
    }

    /// @brief Performs a component-wise addition.
    friend constexpr auto operator+(const Matrix& lhs, const Matrix& rhs) { return lhs.variadicOp(std::plus{}, rhs); }

    /// @brief Performs a component-wise addition.
    friend constexpr auto operator+(T lhs, const Matrix& rhs) { return Matrix{lhs}.variadicOp(std::plus{}, rhs); }

    /// @brief Performs a component-wise addition.
    friend constexpr auto operator+(const Matrix& lhs, T rhs) { return lhs.variadicOp(std::plus{}, Matrix{rhs}); }

    /// @brief Performs a component-wise addition.
    constexpr auto& operator+=(const Matrix& other) { return assignmentOp(std::plus{}, other); }

    /// @brief Performs a component-wise addition.
    constexpr auto& operator+=(T value) { return assignmentOp(std::plus{}, Matrix{value}); }

    /// @brief Performs a component-wise subtraction.
    friend constexpr auto operator-(const Matrix& lhs, const Matrix& rhs) { return lhs.variadicOp(std::minus{}, rhs); }

    /// @brief Performs a component-wise subtraction.
    friend constexpr auto operator-(T lhs, const Matrix& rhs) { return Matrix{lhs}.variadicOp(std::minus{}, rhs); }

    /// @brief Performs a component-wise subtraction.
    friend constexpr auto operator-(const Matrix& lhs, T rhs) { return lhs.variadicOp(std::minus{}, Matrix{rhs}); }

    /// @brief Performs a component-wise subtraction.
    constexpr auto& operator-=(const Matrix& other) { return assignmentOp(std::minus{}, other); }

    /// @brief Performs a component-wise subtraction.
    constexpr auto& operator-=(T value) { return assignmentOp(std::minus{}, Matrix{value}); }

    /// @brief Performs a component-wise multiplication.
    constexpr auto compMul(const Matrix& other) const { return variadicOp(std::multiplies{}, other); }

    /// @brief Performs a component-wise division.
    constexpr auto compDiv(const Matrix& other) const { return variadicOp(std::divides{}, other); }

    /// @brief Performs a component-wise multiplication with the given scalar.
    constexpr auto operator*(T scalar) const { return variadicOp(std::multiplies{}, Matrix{scalar}); }

    /// @brief Performs a component-wise multiplication with the given scalar.
    friend constexpr auto operator*(T scalar, const Matrix& matrix) { return matrix * scalar; }

    /// @brief Performs a component-wise multiplication with the given scalar.
    constexpr auto& operator*=(T scalar) { return assignmentOp(std::multiplies{}, Matrix{scalar}); }

    /// @brief Performs a component-wise division with the given scalar.
    constexpr auto operator/(T scalar) const { return variadicOp(std::divides{}, Matrix{scalar}); }

    /// @brief Performs a component-wise multiplication with the inverse of the matrix.
    friend constexpr std::optional<Matrix> operator/(T scalar, const Matrix& matrix)
    {
        if (auto inv = matrix.inverse())
            return scalar * *inv;
        return std::nullopt;
    }

    /// @brief Performs a component-wise division with the given scalar.
    constexpr auto& operator/=(T scalar) { return assignmentOp(std::divides{}, Matrix{scalar}); }

    /// @brief Performs a matrix-multiplication between the two matrices.
    template <std::size_t v_other_cols>
    friend constexpr auto operator*(const Matrix& lhs, const Matrix<T, v_other_cols, v_cols>& rhs)
    {
        Matrix<T, v_other_cols, v_rows> result;
        sbounds2 bounds{{v_other_cols, v_rows}};
        for (const auto& pos : bounds)
            for (std::size_t i = 0; i < v_cols; i++)
                result[pos] += lhs(i, pos.y()) * rhs(pos.x(), i);
        return result;
    }

    /// @brief Performs a matrix-multiplication with the inverse of rhs.
    friend constexpr std::optional<Matrix> operator/(const Matrix& lhs, const Matrix<T, v_cols, v_cols>& rhs)
    {
        if (auto inv = rhs.inverse())
            return lhs * *inv;
        return std::nullopt;
    }

    /// @brief Performs a matrix-multiplication between the matrix and the given vector, seen as a single-column matrix.
    friend constexpr auto operator*(const Matrix& matrix, const Vector<T, v_cols>& vector)
    {
        return Vector<T, v_rows>{matrix * Matrix<T, 1, v_cols>::fromVector(vector)};
    }

    /// @brief Performs a matrix-multiplication between the transpose of the matrix and the given vector, seen as a
    /// single-column matrix.
    friend constexpr auto operator*(const Vector<T, v_rows>& vector, const Matrix& matrix)
    {
        return matrix.transpose() * vector;
    }

    /// @brief Performs a matrix-multiplication between the inverse of the matrix and the given vector, seen as a
    /// single-column matrix.
    friend constexpr std::optional<Vector<T, v_rows>> operator/(const Vector<T, v_rows>& vector, const Matrix& matrix)
    {
        static_assert(v_cols == v_rows);
        if (auto inv = matrix.inverse())
            return vector * *inv;
        return std::nullopt;
    }

    /// @brief Performs an operation on each component using an arbitrary number of other vectors.
    template <typename TOperation, typename... TMatrices>
    constexpr auto variadicOp(TOperation operation, const TMatrices&... matrices) const
    {
        Matrix<T, v_cols, v_rows> result;
        for (std::size_t i = 0; i < v_cols; i++)
            result[i] = operation((*this)[i], matrices[i]...);
        return result;
    }

    /// @brief Performs an operation with another vector and assigns the result to itself.
    template <typename TOperation>
    constexpr auto& assignmentOp(TOperation operation, const Matrix& other)
    {
        for (std::size_t i = 0; i < v_cols; i++)
            (*this)[i] = operation((*this)[i], other[i]);
        return *this;
    }

    /// @brief Returns a multiline string representing the matrix.
    auto format() const { return (std::stringstream() << *this).str(); }

    /// @brief Appends a string representation of the vector in the form [x, y, z] to the stream.
    friend auto& operator<<(std::ostream& stream, const Matrix& matrix)
    {
        for (std::size_t row = 0; row < v_rows; row++) {
            std::cout << '[';
            if constexpr (v_cols > 0)
                std::cout << matrix(0, row);
            for (std::size_t col = 1; col < v_cols; col++)
                std::cout << ", " << matrix(col, row);
            std::cout << "]\n";
        }
        return stream;
    }
};

template <std::size_t v_cols, std::size_t v_rows = v_cols>
using mat = Matrix<float, v_cols, v_rows>;
using mat2 = mat<2, 2>;
using mat2x3 = mat<2, 3>;
using mat2x4 = mat<2, 4>;
using mat3x2 = mat<3, 2>;
using mat3 = mat<3, 3>;
using mat3x4 = mat<3, 4>;
using mat4x2 = mat<4, 2>;
using mat4x3 = mat<4, 3>;
using mat4 = mat<4, 4>;

template <std::size_t v_cols, std::size_t v_rows = v_cols>
using dmat = Matrix<double, v_cols, v_rows>;
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
