#pragma once

#include "dang-lua/State.h"

#include "dang-math/matrix.h"
#include "dang-math/vector.h"

namespace dang::lua {

template <typename T, std::size_t v_dim>
struct ClassInfo<dang::math::Vector<T, v_dim>> {
    using Vector = dang::math::Vector<T, v_dim>;
    using VectorOrScalar = std::variant<Vector, T>;

    using Swizzled = std::variant<T, dang::math::Vector<T, 2>, dang::math::Vector<T, 3>, dang::math::Vector<T, 4>>;
    using Key = std::variant<std::size_t, std::string_view>;

    using MultiplyType = std::conditional_t<std::is_floating_point_v<T>,
                                            std::variant<T,
                                                         Vector,
                                                         dang::math::Matrix<T, 2, v_dim>,
                                                         dang::math::Matrix<T, 3, v_dim>,
                                                         dang::math::Matrix<T, 4, v_dim>>,
                                            std::variant<T, Vector>>;

    using MultiplyResult = std::conditional_t<
        std::is_floating_point_v<T>,
        std::variant<T,
                     dang::math::Vector<T, 2>,
                     dang::math::Vector<T, 3>,
                     dang::math::Vector<T, 4>,
                     dang::math::Matrix<T, 2, v_dim>,
                     dang::math::Matrix<T, 3, v_dim>,
                     dang::math::Matrix<T, 4, v_dim>>,
        std::variant<T, dang::math::Vector<T, 2>, dang::math::Vector<T, 3>, dang::math::Vector<T, 4>>>;

    using DivideType = std::conditional_t<std::is_floating_point_v<T>,
                                          std::variant<T, Vector, dang::math::Matrix<T, v_dim>>,
                                          std::variant<T, Vector>>;

    using DivideResult =
        std::conditional_t<std::is_floating_point_v<T>,
                           std::variant<T, std::optional<Vector>, std::optional<dang::math::Matrix<T, v_dim>>>,
                           std::variant<T, std::optional<Vector>>>;

    static const std::string base_class_name;

    static const std::string class_name;
    static const std::string class_name_ref;

    static const char* className();
    static const char* classNameRef();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();

    static Arg require(State& lua);

private:
    template <std::size_t... v_indices>
    static auto unpackHelper(const Vector& vector, std::index_sequence<v_indices...>)
    {
        return std::tuple{std::get<v_indices>(vector)...};
    }

    static std::optional<int> axisToIndex(char axis);

    struct Index {
        State& lua;
        const Vector& vector;

        template <std::size_t... v_indices, typename... TSwizzles>
        std::optional<Swizzled> accessHelper(std::index_sequence<v_indices...>, TSwizzles... swizzle) const;

        template <typename... TSwizzles>
        std::optional<Swizzled> access(TSwizzles... swizzle) const;

        std::optional<Swizzled> operator()(std::string_view key) const;
        std::optional<Swizzled> operator()(std::size_t index) const;
    };

    struct NewIndex {
        State& lua;
        Vector& vector;
        const Swizzled& value;

        template <std::size_t... v_indices, typename... TSwizzles>
        void accessHelper(std::index_sequence<v_indices...>, TSwizzles... swizzle) const;

        template <typename... TSwizzles>
        void access(TSwizzles... swizzle) const;

        void operator()(std::string_view key);
        void operator()(std::size_t index);
    };
};

template <typename T, std::size_t v_cols, std::size_t v_rows>
struct ClassInfo<dang::math::Matrix<T, v_cols, v_rows>> {
    using Matrix = dang::math::Matrix<T, v_cols, v_rows>;
    using MatrixOrScalar = std::variant<Matrix, T>;
    using IndexPosOrString = std::variant<std::size_t, dang::math::svec2, const char*>;
    using IndexOrPos = std::variant<std::size_t, dang::math::svec2>;
    using IndexResult = std::variant<std::monostate, T, dang::math::Vector<T, v_rows>>;

    using MultiplyType = std::variant<T,
                                      dang::math::Vector<T, v_cols>,
                                      dang::math::Matrix<T, 2, v_cols>,
                                      dang::math::Matrix<T, 3, v_cols>,
                                      dang::math::Matrix<T, 4, v_cols>>;
    using MultiplyResult = std::variant<T,
                                        dang::math::Vector<T, 2>,
                                        dang::math::Vector<T, 3>,
                                        dang::math::Vector<T, 4>,
                                        dang::math::Matrix<T, 2>,
                                        dang::math::Matrix<T, 2, 3>,
                                        dang::math::Matrix<T, 2, 4>,
                                        dang::math::Matrix<T, 3, 2>,
                                        dang::math::Matrix<T, 3>,
                                        dang::math::Matrix<T, 3, 4>,
                                        dang::math::Matrix<T, 4, 2>,
                                        dang::math::Matrix<T, 4, 3>,
                                        dang::math::Matrix<T, 4>>;

    using DivideType = std::variant<T, dang::math::Matrix<T, v_cols>>;
    using DivideResult =
        std::conditional_t<v_cols == v_rows,
                           std::variant<T, std::optional<Matrix>>,
                           std::variant<T, std::optional<Matrix>, std::optional<dang::math::Matrix<T, v_cols>>>>;

    static const std::string base_class_name;

    static const std::string class_name;
    static const std::string class_name_ref;

    static const char* className();
    static const char* classNameRef();

    static std::vector<luaL_Reg> table();
    static std::vector<luaL_Reg> metatable();

    static Arg require(State& lua);

private:
    static bool columnInRange(std::size_t col);
    static bool rowInRange(std::size_t row);
    static bool inRange(std::size_t col, std::size_t row);
    static bool inRange(dang::math::svec2 pos);

    static void checkColumn(State& lua, std::size_t col, int arg);
    static void checkRow(State& lua, std::size_t row, int arg);
    static void checkRange(State& lua, std::size_t col, std::size_t row, int col_arg, int row_arg);
    static void checkRange(State& lua, dang::math::svec2 pos, int col_arg, int row_arg);

    struct Index {
        const Matrix& matrix;

        IndexResult operator()(std::size_t index) const;
        IndexResult operator()(dang::math::svec2 pos) const;
        IndexResult operator()(const char*) const;
    };

    struct NewIndex {
        State& lua;
        Matrix& matrix;
        Arg value;

        void operator()(std::size_t col);
        void operator()(dang::math::svec2 pos);
    };
};

} // namespace dang::lua

namespace dang::math::lua {

template <typename T>
void requireVector(dang::lua::State& lua, bool global = true);

template <typename T>
void requireMatrix(dang::lua::State& lua, bool global = true);

} // namespace dang::math::lua
