#pragma once

#include "dang-lua/State.h"

#include "vector.h"

namespace dang::lua {

template <typename T, std::size_t Dim>
struct ClassInfo<dmath::Vector<T, Dim>> {
    using Vector = dmath::Vector<T, Dim>;
    using VectorOrScalar = std::variant<dmath::Vector<T, Dim>, T>;
    using Swizzled = std::variant<T, dmath::Vector<T, 2>, dmath::Vector<T, 3>, dmath::Vector<T, 4>>;
    using Key = std::variant<std::size_t, std::string_view>;

    inline static const std::string base_name = [] {
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
            return typeid(T).name() + " vec"s;
    }();

    inline static const std::string name = base_name + std::to_string(Dim);
    inline static const std::string ref_name = name + '&';

    constexpr auto table()
    {
        constexpr auto set = +[](Vector& vec, Args<Dim> values) {
            std::transform(values.begin(), values.end(), vec.begin(), ArgCheck<T>{});
        };

        constexpr auto copy = +[](const Vector& vec) { return vec; };

        constexpr auto unpack = +[](const Vector& vec) { return unpackHelper(vec, std::make_index_sequence<Dim>{}); };

        std::vector result{reg<set>("set"),
                           reg<copy>("copy"),
                           reg<unpack>("unpack"),
                           reg<&Vector::sum>("sum"),
                           reg<&Vector::product>("product"),
                           reg<&Vector::dot>("dot"),
                           reg<&Vector::sqrdot>("sqrdot"),
                           reg<&Vector::vectorTo>("vectorTo"),
                           reg<&Vector::abs>("abs"),
                           reg<&Vector::min>("min"),
                           reg<&Vector::max>("max"),
                           reg<&Vector::reflect>("reflect"),

                           reg<&Vector::allEqualTo>("allEqualTo"),
                           reg<&Vector::anyEqualTo>("anyEqualTo"),
                           reg<&Vector::noneEqualTo>("noneEqualTo"),
                           reg<&Vector::allNotEqualTo>("allNotEqualTo"),
                           reg<&Vector::anyNotEqualTo>("anyNotEqualTo"),
                           reg<&Vector::noneNotEqualTo>("noneNotEqualTo"),
                           reg<&Vector::allLess>("allLess"),
                           reg<&Vector::anyLess>("anyLess"),
                           reg<&Vector::noneLess>("noneLess"),
                           reg<&Vector::allLessEqual>("allLessEqual"),
                           reg<&Vector::anyLessEqual>("anyLessEqual"),
                           reg<&Vector::noneLessEqual>("noneLessEqual"),
                           reg<&Vector::allGreater>("allGreater"),
                           reg<&Vector::anyGreater>("anyGreater"),
                           reg<&Vector::noneGreater>("noneGreater"),
                           reg<&Vector::allGreaterEqual>("allGreaterEqual"),
                           reg<&Vector::anyGreaterEqual>("anyGreaterEqual"),
                           reg<&Vector::noneGreaterEqual>("noneGreaterEqual"),

                           reg<&Vector::format>("format")};

        if constexpr (std::is_floating_point_v<T>) {
            result.push_back(reg<&Vector::normalize>("normalize"));
            result.push_back(reg<&Vector::length>("length"));
            result.push_back(reg<&Vector::floor>("floor"));
            result.push_back(reg<&Vector::ceil>("ceil"));
            result.push_back(reg<&Vector::radToDeg>("radToDeg"));
            result.push_back(reg<&Vector::degToRad>("degToRad"));
            result.push_back(reg<&Vector::distanceTo>("distanceTo"));
            result.push_back(reg<&Vector::cosAngleTo>("cosAngleTo"));
            result.push_back(reg<&Vector::angleRadTo>("angleRadTo"));
            result.push_back(reg<&Vector::angleTo>("angleTo"));

            if constexpr (Dim == 2) {
                constexpr auto cross = +[](const Vector& vec, const std::optional<Vector>& other) {
                    return other ? vec.cross(*other) : vec.cross();
                };
                result.push_back(reg<cross>("cross"));

                constexpr auto slope = +[](const Vector& vec) { return vec.slope(); };
                result.push_back(reg<slope>("slope"));
            }
        }

        if constexpr (Dim == 3) {
            constexpr auto cross = +[](const Vector& lhs, const Vector& rhs) { return lhs.cross(rhs); };
            result.push_back(reg<cross>("cross"));
        }

        return result;
    }

    constexpr auto metatable()
    {
        constexpr auto add = +[](const VectorOrScalar& lhs, const VectorOrScalar& rhs) {
            return std::visit([](const auto& a, const auto& b) -> Vector { return a + b; }, lhs, rhs);
        };
        constexpr auto sub = +[](const VectorOrScalar& lhs, const VectorOrScalar& rhs) {
            return std::visit([](const auto& a, const auto& b) -> Vector { return a - b; }, lhs, rhs);
        };
        constexpr auto mul = +[](const VectorOrScalar& lhs, const VectorOrScalar& rhs) {
            return std::visit([](const auto& a, const auto& b) -> Vector { return a * b; }, lhs, rhs);
        };
        constexpr auto div = +[](const VectorOrScalar& lhs, const VectorOrScalar& rhs) {
            return std::visit([](const auto& a, const auto& b) -> Vector { return a / b; }, lhs, rhs);
        };

        constexpr auto unm = +[](const Vector& vec) { return -vec; };
        constexpr auto len = +[](const Vector&) { return Dim; };

        constexpr auto eq = +[](const Vector& lhs, const Vector& rhs) { return lhs == rhs; };
        constexpr auto lt = +[](const Vector& lhs, const Vector& rhs) { return lhs < rhs; };
        constexpr auto le = +[](const Vector& lhs, const Vector& rhs) { return lhs <= rhs; };

        constexpr auto index = +[](State& lua, const Vector& vec, Key key) { return std::visit(Index{lua, vec}, key); };

        constexpr auto newindex = +[](State& lua, Vector& vec, Key key, const Swizzled& value) {
            std::visit(NewIndex{lua, vec, value}, key);
        };

        constexpr auto pairs = +[](dlua::State& lua, dlua::Arg vector) {
            constexpr auto next = +[](dlua::Arg table, dlua::Arg key) { return table.next(std::move(key)); };
            auto metatable = vector.getMetatable();
            return std::tuple{wrap<next>, metatable ? (*metatable)["__indextable"] : lua.pushNil()};
        };

        return std::array{reg<&Vector::format>("__tostring"),
                          reg<add>("__add"),
                          reg<sub>("__sub"),
                          reg<mul>("__mul"),
                          reg<div>("__div"),
                          reg<unm>("__unm"),
                          reg<len>("__len"),
                          reg<eq>("__eq"),
                          reg<lt>("__lt"),
                          reg<le>("__le"),
                          reg<index>("__index"),
                          reg<newindex>("__newindex"),
                          reg<pairs>("__pairs")};
    }

    static auto require(dlua::State& lua)
    {
        constexpr auto create = +[](dlua::State& lua, dlua::Arg, dlua::VarArgs values) {
            if (values.size() == 0) {
                return Vector();
            }
            else if (values.size() == 1) {
                return Vector(values[0].check<T>());
            }
            else if (values.size() == Dim) {
                Vector result;
                std::transform(values.begin(), values.end(), result.begin(), ArgCheck<T>{});
                return result;
            }
            if constexpr (Dim == 0)
                lua.error("0 parameters expected, got " + std::to_string(values.size()));
            else if constexpr (Dim == 1)
                lua.error("0 or 1 parameters expected, got " + std::to_string(values.size()));
            else
                lua.error("0, 1 or " + std::to_string(Dim) + " parameters expected, got " +
                          std::to_string(values.size()));
        };

        auto result = lua.pushTable();

        if constexpr (Dim == 2) {
            constexpr auto from_slope = +[](std::optional<T> slope) { return Vector::fromSlope(slope); };
            result.rawSetTable("fromSlope", wrap<from_slope>);

            if constexpr (std::is_floating_point_v<T>) {
                constexpr auto from_angle_rad = +[](T radians) { return Vector::fromAngleRad(radians); };
                result.rawSetTable("fromAngleRad", wrap<from_angle_rad>);

                constexpr auto from_angle = +[](T degrees) { return Vector::fromAngle(degrees); };
                result.rawSetTable("fromAngle", wrap<from_angle>);
            }
        }

        auto result_mt = lua.pushTable();
        result_mt.rawSetTable("__call", wrap<create>);

        result.setMetatable(std::move(result_mt));
        return result;
    }

private:
    template <std::size_t... Indices>
    static auto unpackHelper(const Vector& vector, std::index_sequence<Indices...>)
    {
        return std::tuple{std::get<Indices>(vector)...};
    }

    static std::optional<int> axisToIndex(char axis)
    {
        if constexpr (Dim >= 1 && Dim <= 4)
            if (axis == 'x')
                return 0;
        if constexpr (Dim >= 2 && Dim <= 4)
            if (axis == 'y')
                return 1;
        if constexpr (Dim >= 3 && Dim <= 4)
            if (axis == 'z')
                return 2;
        if constexpr (Dim >= 4 && Dim <= 4)
            if (axis == 'w')
                return 3;
        return std::nullopt;
    }

    struct Index {
        State& lua;
        const Vector& vector;

        template <std::size_t... Indices, typename... TSwizzles>
        std::optional<Swizzled> accessHelper(std::index_sequence<Indices...>, TSwizzles... swizzle) const
        {
            auto indices = std::array{axisToIndex(swizzle)...};
            if ((!std::get<Indices>(indices) || ...))
                return std::nullopt;

            if constexpr (sizeof...(TSwizzles) == 1)
                return vector[*std::get<0>(indices)];
            else
                return dmath::Vector<T, sizeof...(TSwizzles)>(vector[*std::get<Indices>(indices)]...);
        }

        template <typename... TSwizzles>
        std::optional<Swizzled> access(TSwizzles... swizzle) const
        {
            return accessHelper(std::index_sequence_for<TSwizzles...>{}, swizzle...);
        }

        std::optional<Swizzled> operator()(std::string_view key) const
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

        std::optional<Swizzled> operator()(std::size_t index) const
        {
            if (index >= 1 && index <= Dim)
                return vector[index - 1];
            return std::nullopt;
        }
    };

    struct NewIndex {
        State& lua;
        Vector& vector;
        const Swizzled& value;

        template <std::size_t... Indices, typename... TSwizzles>
        void accessHelper(std::index_sequence<Indices...>, TSwizzles... swizzle) const
        {
            auto indices = std::array{axisToIndex(swizzle)...};
            if ((!std::get<Indices>(indices) || ...))
                lua.argError(2, "invalid swizzle");

            if (auto opt_value = std::get_if<T>(&value)) {
                ((vector[*std::get<Indices>(indices)] = *opt_value), ...);
                return;
            }

            if constexpr (sizeof...(TSwizzles) > 1) {
                if (auto opt_values = std::get_if<dmath::Vector<T, sizeof...(TSwizzles)>>(&value)) {
                    ((vector[*std::get<Indices>(indices)] = (*opt_values)[Indices]), ...);
                    return;
                }
            }

            lua.argError(2, "swizzle mismatch");
        }

        template <typename... TSwizzles>
        void access(TSwizzles... swizzle) const
        {
            accessHelper(std::index_sequence_for<TSwizzles...>{}, swizzle...);
        }

        void operator()(std::string_view key)
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

        void operator()(std::size_t index)
        {
            if (index < 1 || index > Dim)
                lua.argError(2, "index out of range");
            if (auto opt_value = std::get_if<T>(&value))
                vector[index - 1] = *opt_value;
            else
                lua.argError(2, "single value expected, got vector");
        }
    };
};

template <typename T, std::size_t Dim>
const char* ClassName<dmath::Vector<T, Dim>> = ClassInfo<dmath::Vector<T, Dim>>::name.c_str();

template <typename T, std::size_t Dim>
const char* ClassNameRef<dmath::Vector<T, Dim>> = ClassInfo<dmath::Vector<T, Dim>>::ref_name.c_str();

} // namespace dang::lua
