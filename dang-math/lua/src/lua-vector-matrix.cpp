#include "dang-math/lua-vector-matrix.h"

#include "dang-math/lua-enums.h"

#include "dang-utils/utils.h"

namespace dang::lua {

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Vector<T, v_dim>>::base_class_name = [] {
    using namespace std::literals;
    if constexpr (std::is_same_v<T, float>)
        return "vec"s;
    else if constexpr (std::is_same_v<T, double>)
        return "dvec"s;
    else if constexpr (std::is_same_v<T, int>)
        return "ivec"s;
    else if constexpr (std::is_same_v<T, unsigned>)
        return "uvec"s;
    else if constexpr (std::is_same_v<T, std::size_t>)
        return "svec"s;
    else if constexpr (std::is_same_v<T, bool>)
        return "bvec"s;
    else
        static_assert(dutils::always_false_v<T>, "unsupported vector type");
}();

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Vector<T, v_dim>>::class_name =
    ClassInfo<dang::math::Vector<T, v_dim>>::base_class_name + std::to_string(v_dim);

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Vector<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Vector<T, v_dim>>::table()
{
    constexpr auto type = +[](State& lua) { return lua.pushRequire<Vector>(false); };

    constexpr auto set = +[](Vector& vec, Args<v_dim> values) {
        std::transform(values.begin(), values.end(), vec.begin(), ArgCheck<T>{});
    };

    constexpr auto copy = +[](const Vector& vec) { return vec; };

    constexpr auto unpack = +[](const Vector& vec) { return unpackHelper(vec, std::make_index_sequence<v_dim>{}); };

    std::vector result{reg<type>("type"),
                       reg<set>("set"),
                       reg<copy>("copy"),
                       reg<unpack>("unpack"),
                       reg<&Vector::lessThan>("lessThan"),
                       reg<&Vector::lessThanEqual>("lessThanEqual"),
                       reg<&Vector::greaterThan>("greaterThan"),
                       reg<&Vector::greaterThanEqual>("greaterThanEqual"),
                       reg<&Vector::equal>("equal"),
                       reg<&Vector::notEqual>("notEqual"),
                       reg<&Vector::format>("format"),
                       reg<&Vector::mirroredSwizzle>("mirroredSwizzle")};

    if constexpr (std::is_floating_point_v<T>) {
        result.push_back(reg<&Vector::length>("length"));
        result.push_back(reg<&Vector::normalize>("normalize"));
        result.push_back(reg<&Vector::distanceTo>("distanceTo"));
        result.push_back(reg<&Vector::cosAngleTo>("cosAngleTo"));
        result.push_back(reg<&Vector::radiansTo>("radiansTo"));
        result.push_back(reg<&Vector::degreesTo>("degreesTo"));
        result.push_back(reg<&Vector::radians>("radians"));
        result.push_back(reg<&Vector::degrees>("degrees"));
        result.push_back(reg<&Vector::floor>("floor"));
        result.push_back(reg<&Vector::ceil>("ceil"));

        if constexpr (v_dim == 2) {
            constexpr auto cross = +[](const Vector& vec, const std::optional<Vector>& other) {
                return other ? vec.cross(*other) : vec.cross();
            };
            result.push_back(reg<cross>("cross"));

            constexpr auto slope = +[](const Vector& vec) { return vec.slope(); };
            result.push_back(reg<slope>("slope"));
        }
    }

    if constexpr (std::is_same_v<T, bool>) {
        result.push_back(reg<&Vector::all>("all"));
        result.push_back(reg<&Vector::any>("any"));
        result.push_back(reg<&Vector::none>("none"));
        result.push_back(reg<&Vector::invert>("invert"));
    }
    else {
        result.push_back(reg<&Vector::sum>("sum"));
        result.push_back(reg<&Vector::product>("product"));
        result.push_back(reg<&Vector::dot>("dot"));
        result.push_back(reg<&Vector::sqrdot>("sqrdot"));
        result.push_back(reg<&Vector::vectorTo>("vectorTo"));
        result.push_back(reg<&Vector::min>("min"));
        result.push_back(reg<&Vector::max>("max"));
        result.push_back(reg<&Vector::minAxis>("minAxis"));
        result.push_back(reg<&Vector::maxAxis>("maxAxis"));
        result.push_back(reg<&Vector::minMaxAxis>("minMaxAxis"));
        result.push_back(reg<&Vector::minValue>("minValue"));
        result.push_back(reg<&Vector::maxValue>("maxValue"));
        result.push_back(reg<&Vector::minMaxValue>("minMaxValue"));
        result.push_back(reg<&Vector::clamp>("clamp"));
        result.push_back(reg<&Vector::reflect>("reflect"));

        if constexpr (v_dim == 3) {
            constexpr auto cross = +[](const Vector& lhs, const Vector& rhs) { return lhs.cross(rhs); };
            result.push_back(reg<cross>("cross"));
        }

        if constexpr (std::is_signed_v<T>) {
            result.push_back(reg<&Vector::abs>("abs"));
        }
    }

    return result;
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Vector<T, v_dim>>::metatable()
{
    constexpr auto len = +[](const Vector&) { return v_dim; };

    constexpr auto eq = +[](const Vector& lhs, const Vector& rhs) { return lhs == rhs; };
    constexpr auto lt = +[](const Vector& lhs, const Vector& rhs) { return lhs < rhs; };
    constexpr auto le = +[](const Vector& lhs, const Vector& rhs) { return lhs <= rhs; };

    constexpr auto index = +[](State& lua, const Vector& vec, Key key) { return std::visit(Index{lua, vec}, key); };
    constexpr auto newindex = +[](State& lua, Vector& vec, Key key, Arg value) {
        std::visit(NewIndex{lua, vec, value}, key);
    };

    std::vector result{reg<&Vector::format>("__tostring"),
                       reg<len>("__len"),
                       reg<eq>("__eq"),
                       reg<lt>("__lt"),
                       reg<le>("__le"),
                       reg<index>("__index"),
                       reg<newindex>("__newindex"),
                       reg<indextable_pairs>("__pairs")};

    if constexpr (!std::is_same_v<T, bool>) {
        constexpr auto add = +[](const VectorOrScalar& lhs, const VectorOrScalar& rhs) {
            return std::visit([](const auto& a, const auto& b) -> VectorOrScalar { return a + b; }, lhs, rhs);
        };
        constexpr auto sub = +[](const VectorOrScalar& lhs, const VectorOrScalar& rhs) {
            return std::visit([](const auto& a, const auto& b) -> VectorOrScalar { return a - b; }, lhs, rhs);
        };
        constexpr auto mul = +[](const VectorOrScalar& lhs, const MultiplyType& rhs) {
            return std::visit([](const auto& a, const auto& b) -> MultiplyResult { return a * b; }, lhs, rhs);
        };
        constexpr auto div = +[](const VectorOrScalar& lhs, const DivideType& rhs) {
            return std::visit([](const auto& a, const auto& b) -> DivideResult { return a / b; }, lhs, rhs);
        };

        result.push_back(reg<add>("__add"));
        result.push_back(reg<sub>("__sub"));
        result.push_back(reg<mul>("__mul"));
        result.push_back(reg<div>("__div"));

        if constexpr (std::is_signed_v<T>) {
            constexpr auto unm = +[](const Vector& vec) { return -vec; };
            result.push_back(reg<unm>("__unm"));
        }

        if constexpr (std::is_integral_v<T>) {
            constexpr auto bnot = +[](const Vector& vec) { return ~vec; };
            constexpr auto band = +[](const Vector& lhs, const Vector& rhs) { return lhs & rhs; };
            constexpr auto bor = +[](const Vector& lhs, const Vector& rhs) { return lhs | rhs; };
            constexpr auto bxor = +[](const Vector& lhs, const Vector& rhs) { return lhs ^ rhs; };

            result.push_back(reg<bnot>("__bnot"));
            result.push_back(reg<band>("__band"));
            result.push_back(reg<bor>("__bor"));
            result.push_back(reg<bxor>("__bxor"));
        }
    }

    return result;
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Vector<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs values) {
        if (values.size() == 0)
            return Vector();
        if (values.size() == 1)
            return Vector(values[0].check<T>());
        if (values.size() == v_dim) {
            Vector result;
            std::transform(values.begin(), values.end(), result.begin(), ArgCheck<T>{});
            return result;
        }

        if constexpr (v_dim == 0)
            lua.error("0 arguments expected, got " + std::to_string(values.size()));
        else if constexpr (v_dim == 1)
            lua.error("0 or 1 arguments expected, got " + std::to_string(values.size()));
        else
            lua.error("0, 1 or " + std::to_string(v_dim) + " arguments expected, got " + std::to_string(values.size()));
    };

    auto result = lua.pushTable();

    if constexpr (v_dim >= 1) {
        Vector vec;
        vec.x() = 1;
        result.setTable("x", vec);
    }
    if constexpr (v_dim >= 2) {
        Vector vec;
        vec.y() = 1;
        result.setTable("y", vec);
    }
    if constexpr (v_dim >= 3) {
        Vector vec;
        vec.z() = 1;
        result.setTable("z", vec);
    }
    if constexpr (v_dim >= 4) {
        Vector vec;
        vec.w() = 1;
        result.setTable("w", vec);
    }

    if constexpr (v_dim == 2) {
        if constexpr (!std::is_same_v<T, bool>) {
            constexpr auto from_slope = +[](std::optional<T> slope) { return Vector::fromSlope(slope); };
            result.setTable("fromSlope", wrap<from_slope>);
        }

        if constexpr (std::is_floating_point_v<T>) {
            constexpr auto from_angle_rad = +[](T radians) { return Vector::fromRadians(radians); };
            result.setTable("fromRadians", wrap<from_angle_rad>);

            constexpr auto from_angle = +[](T degrees) { return Vector::fromDegrees(degrees); };
            result.setTable("fromDegrees", wrap<from_angle>);
        }
    }

    auto result_mt = lua.pushTable();
    result_mt.setTable("__call", wrap<create>);

    result.setMetatable(std::move(result_mt));
    return result;
}

template <typename T, std::size_t v_dim>
std::optional<int> ClassInfo<dang::math::Vector<T, v_dim>>::axisToIndex(char axis)
{
    if constexpr (v_dim >= 1 && v_dim <= 4)
        if (axis == 'x')
            return 0;
    if constexpr (v_dim >= 2 && v_dim <= 4)
        if (axis == 'y')
            return 1;
    if constexpr (v_dim >= 3 && v_dim <= 4)
        if (axis == 'z')
            return 2;
    if constexpr (v_dim >= 4 && v_dim <= 4)
        if (axis == 'w')
            return 3;
    return std::nullopt;
}

template <typename T, std::size_t v_dim>
template <std::size_t... v_indices, typename... TSwizzles>
std::optional<typename ClassInfo<dang::math::Vector<T, v_dim>>::Swizzled> ClassInfo<
    dang::math::Vector<T, v_dim>>::Index::accessHelper(std::index_sequence<v_indices...>, TSwizzles... swizzle) const
{
    auto indices = std::array{axisToIndex(swizzle)...};
    if ((!std::get<v_indices>(indices) || ...))
        return std::nullopt;

    if constexpr (sizeof...(TSwizzles) == 1)
        return vector[*std::get<0>(indices)];
    else
        return dang::math::Vector<T, sizeof...(TSwizzles)>(vector[*std::get<v_indices>(indices)]...);
}

template <typename T, std::size_t v_dim>
template <typename... TSwizzles>
std::optional<typename ClassInfo<dang::math::Vector<T, v_dim>>::Swizzled> ClassInfo<
    dang::math::Vector<T, v_dim>>::Index::access(TSwizzles... swizzle) const
{
    return accessHelper(std::index_sequence_for<TSwizzles...>{}, swizzle...);
}

template <typename T, std::size_t v_dim>
std::optional<typename ClassInfo<dang::math::Vector<T, v_dim>>::Swizzled> ClassInfo<
    dang::math::Vector<T, v_dim>>::Index::operator()(std::string_view key) const
{
    switch (key.size()) {
    case 1:
        return access(key[0]);
    case 2:
        return access(key[0], key[1]);
    case 3:
        return access(key[0], key[1], key[2]);
    case 4:
        return access(key[0], key[1], key[2], key[3]);
    }
    return std::nullopt;
}

template <typename T, std::size_t v_dim>
std::optional<typename ClassInfo<dang::math::Vector<T, v_dim>>::Swizzled> ClassInfo<
    dang::math::Vector<T, v_dim>>::Index::operator()(std::size_t index) const
{
    if (index >= 1 && index <= v_dim)
        return vector[index - 1];
    return std::nullopt;
}

template <typename T, std::size_t v_dim>
template <std::size_t... v_indices, typename... TSwizzles>
void ClassInfo<dang::math::Vector<T, v_dim>>::NewIndex::accessHelper(std::index_sequence<v_indices...>,
                                                                     TSwizzles... swizzle)
{
    auto indices = std::array{axisToIndex(swizzle)...};
    if ((!std::get<v_indices>(indices) || ...))
        lua.argError(2, "invalid swizzle");

    if (auto opt_value = value.to<T>()) {
        ((vector[*std::get<v_indices>(indices)] = *opt_value), ...);
        return;
    }

    if constexpr (sizeof...(TSwizzles) > 1) {
        if (auto opt_values = value.to<dang::math::Vector<T, sizeof...(TSwizzles)>>()) {
            ((vector[*std::get<v_indices>(indices)] = opt_values->get()[v_indices]), ...);
            return;
        }
    }

    lua.argError(2, "swizzle mismatch");
}

template <typename T, std::size_t v_dim>
template <typename... TSwizzles>
void ClassInfo<dang::math::Vector<T, v_dim>>::NewIndex::access(TSwizzles... swizzle)
{
    accessHelper(std::index_sequence_for<TSwizzles...>{}, swizzle...);
}

template <typename T, std::size_t v_dim>
void ClassInfo<dang::math::Vector<T, v_dim>>::NewIndex::operator()(std::string_view key)
{
    switch (key.size()) {
    case 1:
        access(key[0]);
        return;
    case 2:
        access(key[0], key[1]);
        return;
    case 3:
        access(key[0], key[1], key[2]);
        return;
    case 4:
        access(key[0], key[1], key[2], key[3]);
        return;
    }
    lua.argError(2, "invalid swizzle");
}

template <typename T, std::size_t v_dim>
void ClassInfo<dang::math::Vector<T, v_dim>>::NewIndex::operator()(std::size_t index)
{
    if (index < 1 || index > v_dim)
        lua.argError(2, "index out of range");
    if (auto opt_value = value.to<T>())
        vector[index - 1] = *opt_value;
    else
        lua.argError(2, "single value expected, got vector");
}

template struct ClassInfo<dang::math::Vector<float, 2>>;
template struct ClassInfo<dang::math::Vector<float, 3>>;
template struct ClassInfo<dang::math::Vector<float, 4>>;

template struct ClassInfo<dang::math::Vector<double, 2>>;
template struct ClassInfo<dang::math::Vector<double, 3>>;
template struct ClassInfo<dang::math::Vector<double, 4>>;

template struct ClassInfo<dang::math::Vector<int, 2>>;
template struct ClassInfo<dang::math::Vector<int, 3>>;
template struct ClassInfo<dang::math::Vector<int, 4>>;

template struct ClassInfo<dang::math::Vector<unsigned, 2>>;
template struct ClassInfo<dang::math::Vector<unsigned, 3>>;
template struct ClassInfo<dang::math::Vector<unsigned, 4>>;

template struct ClassInfo<dang::math::Vector<std::size_t, 2>>;
template struct ClassInfo<dang::math::Vector<std::size_t, 3>>;
template struct ClassInfo<dang::math::Vector<std::size_t, 4>>;

template struct ClassInfo<dang::math::Vector<bool, 2>>;
template struct ClassInfo<dang::math::Vector<bool, 3>>;
template struct ClassInfo<dang::math::Vector<bool, 4>>;

template <typename T, std::size_t v_cols, std::size_t v_rows>
const std::string ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::base_class_name = [] {
    using namespace std::literals;
    if constexpr (std::is_same_v<T, float>)
        return "mat"s;
    else if constexpr (std::is_same_v<T, double>)
        return "dmat"s;
    else
        static_assert(dutils::always_false_v<T>, "unsupported matrix type");
}();

template <typename T, std::size_t v_cols, std::size_t v_rows>
const std::string ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::class_name =
    ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::base_class_name + std::to_string(v_cols) +
    (v_cols != v_rows ? 'x' + std::to_string(v_rows) : "");

template <typename T, std::size_t v_cols, std::size_t v_rows>
const char* ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
std::vector<luaL_Reg> ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::table()
{
    constexpr auto type = +[](State& lua) { return lua.pushRequire<Matrix>(false); };

    constexpr auto set = +[](Matrix& mat, Args<v_cols> values) {
        std::transform(values.begin(), values.end(), mat.begin(), ArgCheck<dang::math::Vector<T, v_rows>>{});
    };

    constexpr auto copy = +[](const Matrix& mat) { return mat; };

    constexpr auto get_at = +[](const Matrix& mat, std::size_t col, std::size_t row) -> std::optional<T> {
        if (inRange(col, row))
            return mat(col - 1, row - 1);
        return std::nullopt;
    };

    constexpr auto set_at = +[](State& lua, Matrix& mat, std::size_t col, std::size_t row, T value) {
        checkRange(lua, col, row, 2, 3);
        mat(col - 1, row - 1) = value;
    };

    constexpr auto cofactor_at = +[](State& lua, const Matrix& mat, std::size_t col, std::size_t row) {
        checkRange(lua, col, row, 2, 3);
        return mat.cofactor(col - 1, row - 1);
    };

    constexpr auto cofactor = +[](State& lua, const Matrix& mat, dang::math::svec2 pos) {
        checkRange(lua, pos, 2, 2);
        return mat.cofactor(pos);
    };

    std::vector result{reg<type>("type"),
                       reg<set>("set"),
                       reg<copy>("copy"),
                       reg<get_at>("getAt"),
                       reg<set_at>("setAt"),
                       reg<&Matrix::format>("format"),
                       reg<&Matrix::transpose>("transpose"),
                       reg<cofactor_at>("cofactorAt"),
                       reg<cofactor>("cofactor"),
                       reg<&Matrix::cofactorMatrix>("cofactorMatrix"),
                       reg<&Matrix::adjugate>("adjugate"),
                       reg<&Matrix::determinant>("determinant"),
                       reg<&Matrix::solvable>("solvable"),
                       reg<&Matrix::compMul>("compMul"),
                       reg<&Matrix::compDiv>("compDiv")};

    // Matrix parameters for solve functions are not const, to enable in-place calculation
    // (swaps columns around, but reverts to the original at the end)

    if constexpr (v_cols == v_rows + 1) {
        constexpr auto solve_col = +[](State& lua, Matrix& mat, std::size_t col) {
            checkColumn(lua, col, 2);
            return mat.solveCol(col - 1);
        };
        result.push_back(reg<solve_col>("solveCol"));

        constexpr auto solve = +[](Matrix& mat) { return mat.solve(); };
        result.push_back(reg<solve>("solve"));
    }

    if constexpr (v_cols == v_rows) {
        result.push_back(reg<&Matrix::inverse>("inverse"));

        constexpr auto solve_col =
            +[](State& lua, Matrix& mat, std::size_t col, const dang::math::Vector<T, v_cols>& vec) {
                checkColumn(lua, col, 2);
                return mat.solveCol(col - 1, vec);
            };
        result.push_back(reg<solve_col>("solveCol"));

        constexpr auto solve = +[](Matrix& mat, const dang::math::Vector<T, v_cols>& vec) { return mat.solve(vec); };
        result.push_back(reg<solve>("solve"));
    }
    return result;
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
std::vector<luaL_Reg> ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::metatable()
{
    constexpr auto add = +[](const MatrixOrScalar& lhs, const MatrixOrScalar& rhs) {
        return std::visit([](const auto& a, const auto& b) -> MatrixOrScalar { return a + b; }, lhs, rhs);
    };
    constexpr auto sub = +[](const MatrixOrScalar& lhs, const MatrixOrScalar& rhs) {
        return std::visit([](const auto& a, const auto& b) -> MatrixOrScalar { return a - b; }, lhs, rhs);
    };
    constexpr auto mul = +[](const MatrixOrScalar& lhs, const MultiplyType& rhs) {
        return std::visit([](const auto& a, const auto& b) -> MultiplyResult { return a * b; }, lhs, rhs);
    };
    constexpr auto div = +[](const MatrixOrScalar& lhs, const DivideType& rhs) {
        return std::visit([](const auto& a, const auto& b) -> DivideResult { return a / b; }, lhs, rhs);
    };

    constexpr auto len = +[](const Matrix&) { return v_cols; };

    constexpr auto eq = +[](const Matrix& lhs, const Matrix& rhs) { return lhs == rhs; };
    constexpr auto lt = +[](const Matrix& lhs, const Matrix& rhs) { return lhs < rhs; };
    constexpr auto le = +[](const Matrix& lhs, const Matrix& rhs) { return lhs <= rhs; };

    constexpr auto index = +[](const Matrix& mat, const IndexPosOrString& key) { return std::visit(Index{mat}, key); };
    constexpr auto newindex = +[](State& lua, Matrix& mat, const IndexOrPos& key, Arg value) {
        std::visit(NewIndex{lua, mat, value}, key);
    };

    std::vector result{reg<&Matrix::format>("__tostring"),
                       reg<add>("__add"),
                       reg<sub>("__sub"),
                       reg<mul>("__mul"),
                       reg<div>("__div"),
                       reg<len>("__len"),
                       reg<eq>("__eq"),
                       reg<lt>("__lt"),
                       reg<le>("__le"),
                       reg<index>("__index"),
                       reg<newindex>("__newindex"),
                       reg<indextable_pairs>("__pairs")};

    if constexpr (std::is_signed_v<T>) {
        constexpr auto unm = +[](const Matrix& mat) { return -mat; };
        result.push_back(reg<unm>("__unm"));
    }

    return result;
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
Arg ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs values) {
        if (values.size() == 0)
            return Matrix();
        if (values.size() == 1)
            return Matrix(values[0].check<T>());
        if (values.size() == v_cols * v_rows) {
            Matrix result;
            dang::math::sbounds2 bounds{{v_cols, v_rows}};
            for (const auto& [col, row] : bounds)
                result(col, row) = values[static_cast<int>(col * v_rows + row)].check<T>();
            return result;
        }
        lua.error("0, 1 or " + std::to_string(v_cols * v_rows) + " arguments expected, got " +
                  std::to_string(values.size()));
    };

    constexpr auto identity =
        +[](std::optional<T> value) { return value ? Matrix::identity(*value) : Matrix::identity(); };

    auto result = lua.pushTable();
    result.setTable("identity", wrap<identity>);

    auto result_mt = lua.pushTable();
    result_mt.setTable("__call", wrap<create>);

    result.setMetatable(std::move(result_mt));
    return result;
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
bool ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::columnInRange(std::size_t col)
{
    return col >= 1 && col <= v_cols;
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
bool ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::rowInRange(std::size_t row)
{
    return row >= 1 && row <= v_rows;
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
bool ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::inRange(std::size_t col, std::size_t row)
{
    return columnInRange(col) && rowInRange(row);
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
bool ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::inRange(dang::math::svec2 pos)
{
    return inRange(pos.x(), pos.y());
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
void ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::checkColumn(State& lua, std::size_t col, int arg)
{
    if (col < 1 || col > v_cols)
        lua.argError(arg, "column out of range");
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
void ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::checkRow(State& lua, std::size_t row, int arg)
{
    if (row < 1 || row > v_rows)
        lua.argError(arg, "row out of range");
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
void ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::checkRange(
    State& lua, std::size_t col, std::size_t row, int col_arg, int row_arg)
{
    checkColumn(lua, col, col_arg);
    checkRow(lua, row, row_arg);
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
void ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::checkRange(State& lua,
                                                                  dang::math::svec2 pos,
                                                                  int col_arg,
                                                                  int row_arg)
{
    checkRange(lua, pos.x(), pos.y(), col_arg, row_arg);
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
typename ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::IndexResult ClassInfo<
    dang::math::Matrix<T, v_cols, v_rows>>::Index::operator()(std::size_t index) const
{
    if (index >= 1 && index <= v_cols)
        return matrix[index - 1];
    return {};
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
typename ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::IndexResult ClassInfo<
    dang::math::Matrix<T, v_cols, v_rows>>::Index::operator()(dang::math::svec2 pos) const
{
    if (pos.greaterThanEqual(1).all() && pos.lessThanEqual({v_cols, v_rows}).all())
        return matrix[pos - 1];
    return {};
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
typename ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::IndexResult ClassInfo<
    dang::math::Matrix<T, v_cols, v_rows>>::Index::operator()(const char*) const
{
    return {};
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
void ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::NewIndex::operator()(std::size_t col)
{
    checkColumn(lua, col, 2);
    matrix[col - 1] = value.check<dang::math::Vector<T, v_rows>>();
}

template <typename T, std::size_t v_cols, std::size_t v_rows>
void ClassInfo<dang::math::Matrix<T, v_cols, v_rows>>::NewIndex::operator()(dang::math::svec2 pos)
{
    checkRange(lua, pos, 2, 2);
    matrix[pos - 1] = value.check<T>();
}

template struct ClassInfo<dang::math::Matrix<float, 2, 2>>;
template struct ClassInfo<dang::math::Matrix<float, 2, 3>>;
template struct ClassInfo<dang::math::Matrix<float, 2, 4>>;
template struct ClassInfo<dang::math::Matrix<float, 3, 2>>;
template struct ClassInfo<dang::math::Matrix<float, 3, 3>>;
template struct ClassInfo<dang::math::Matrix<float, 3, 4>>;
template struct ClassInfo<dang::math::Matrix<float, 4, 2>>;
template struct ClassInfo<dang::math::Matrix<float, 4, 3>>;
template struct ClassInfo<dang::math::Matrix<float, 4, 4>>;

template struct ClassInfo<dang::math::Matrix<double, 2, 2>>;
template struct ClassInfo<dang::math::Matrix<double, 2, 3>>;
template struct ClassInfo<dang::math::Matrix<double, 2, 4>>;
template struct ClassInfo<dang::math::Matrix<double, 3, 2>>;
template struct ClassInfo<dang::math::Matrix<double, 3, 3>>;
template struct ClassInfo<dang::math::Matrix<double, 3, 4>>;
template struct ClassInfo<dang::math::Matrix<double, 4, 2>>;
template struct ClassInfo<dang::math::Matrix<double, 4, 3>>;
template struct ClassInfo<dang::math::Matrix<double, 4, 4>>;

} // namespace dang::lua

namespace dang::math::lua {

template <typename T>
void requireVector(dang::lua::State& lua, bool global)
{
    lua.require<Vector<T, 2>>(global);
    lua.require<Vector<T, 3>>(global);
    lua.require<Vector<T, 4>>(global);
}

template <typename T>
void requireMatrix(dang::lua::State& lua, bool global)
{
    lua.require<Matrix<T, 2, 2>>(global);
    lua.require<Matrix<T, 2, 3>>(global);
    lua.require<Matrix<T, 2, 4>>(global);
    lua.require<Matrix<T, 3, 2>>(global);
    lua.require<Matrix<T, 3, 3>>(global);
    lua.require<Matrix<T, 3, 4>>(global);
    lua.require<Matrix<T, 4, 2>>(global);
    lua.require<Matrix<T, 4, 3>>(global);
    lua.require<Matrix<T, 4, 4>>(global);
}

template void requireVector<float>(dang::lua::State&, bool);
template void requireVector<double>(dang::lua::State&, bool);
template void requireVector<int>(dang::lua::State&, bool);
template void requireVector<unsigned>(dang::lua::State&, bool);
template void requireVector<std::size_t>(dang::lua::State&, bool);
template void requireVector<bool>(dang::lua::State&, bool);

template void requireMatrix<float>(dang::lua::State&, bool);
template void requireMatrix<double>(dang::lua::State&, bool);

} // namespace dang::math::lua
