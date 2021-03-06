#include "dang-math/lua-geometry.h"

#include "dang-math/lua-vector-matrix.h"

#include "dang-utils/utils.h"

namespace dang::lua {

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Line<T, v_dim>>::base_class_name = [] {
    using namespace std::literals;
    if constexpr (std::is_same_v<T, float>)
        return "Line"s;
    else if constexpr (std::is_same_v<T, double>)
        return "DLine"s;
    else
        static_assert(dutils::always_false_v<T>, "unsupported Line type");
}();

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Line<T, v_dim>>::class_name =
    ClassInfo<dang::math::Line<T, v_dim>>::base_class_name + std::to_string(v_dim);

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Line<T, v_dim>>::class_name_ref;

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Line<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Line<T, v_dim>>::classNameRef()
{
    return class_name_ref.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Line<T, v_dim>>::table()
{
    constexpr auto direction = +[](const Line& line) { return line.direction(); };
    constexpr auto setDirection = +[](Line& line, const Vector& direction) { line.direction() = direction; };
    constexpr auto head = +[](const Line& line) { return line.head(); };
    constexpr auto setHead = +[](Line& line, const Vector& head) { line.setHead(head); };
    constexpr auto tail = +[](const Line& line) { return line.tail(); };
    constexpr auto setTail = +[](Line& line, const Vector& tail) { line.setTail(tail); };
    constexpr auto length = +[](const Line& line) { return line.length(); };
    constexpr auto orthoProj = +[](const Line& line, const Vector& point) { return line.orthoProj(point); };

    std::vector result{reg<direction>("direction"),
                       reg<setDirection>("setDirection"),
                       reg<head>("head"),
                       reg<setHead>("setHead"),
                       reg<tail>("tail"),
                       reg<setTail>("setTail"),
                       reg<length>("length"),
                       reg<orthoProj>("orthoProj")};

    if constexpr (v_dim == 2) {
        result.push_back(reg<&Line::distanceTo>("distanceTo"));
        result.push_back(reg<&Line::sideOf>("sideOf"));
        result.push_back(reg<&Line::intersectionMatrix>("intersectionMatrix"));
        result.push_back(reg<&Line::intersectionFactor>("intersectionFactor"));
        result.push_back(reg<&Line::intersectionFactors>("intersectionFactors"));
        result.push_back(reg<&Line::intersectionPoint>("intersectionPoint"));
    }

    return result;
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Line<T, v_dim>>::metatable()
{
    constexpr auto index = +[](const Line& line, std::variant<T, const char*> factor) {
        return std::visit(dutils::Overloaded{[&](T factor) { return std::optional(line[factor]); },
                                             [](...) { return std::optional<Vector>(); }},
                          factor);
    };
    constexpr auto eq = +[](const Line& lhs, const Line& rhs) { return lhs == rhs; };
    constexpr auto pairs = +[](State& lua, Arg line) {
        constexpr auto next = +[](State& lua, Arg table, Arg key) {
            auto result = table.next(std::move(key));
            return result ? VarArgs(*result) : VarArgs(lua.pushNil());
        };
        return std::tuple{wrap<next>, line.getMetafield("indextable")};
    };

    return std::vector{reg<index>("__index"), reg<eq>("__eq"), reg<pairs>("__pairs")};
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Line<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs args) {
        if (args.empty())
            return Line();
        auto support = args[0].check<Vector>();
        auto direction = args[1].check<Vector>();
        return Line(support, direction);
    };

    auto result = lua.pushTable();

    auto result_mt = lua.pushTable();
    result_mt.setTable("__call", wrap<create>);

    result.setMetatable(std::move(result_mt));
    return result;
}

template struct ClassInfo<dang::math::Line<float, 2>>;
template struct ClassInfo<dang::math::Line<float, 3>>;

template struct ClassInfo<dang::math::Line<double, 2>>;
template struct ClassInfo<dang::math::Line<double, 3>>;

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Plane<T, v_dim>>::base_class_name = [] {
    using namespace std::literals;
    if constexpr (std::is_same_v<T, float>)
        return "Plane"s;
    else if constexpr (std::is_same_v<T, double>)
        return "DPlane"s;
    else
        static_assert(dutils::always_false_v<T>, "unsupported Plane type");
}();

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Plane<T, v_dim>>::class_name =
    ClassInfo<dang::math::Plane<T, v_dim>>::base_class_name + std::to_string(v_dim);

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Plane<T, v_dim>>::class_name_ref =
    ClassInfo<dang::math::Plane<T, v_dim>>::class_name + '&';

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Plane<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Plane<T, v_dim>>::classNameRef()
{
    return class_name_ref.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Plane<T, v_dim>>::table()
{
    constexpr auto at = +[](const Plane& plane, T x, T y) { return plane[{x, y}]; };
    constexpr auto line = +[](State& lua, const Plane& plane, std::size_t index) -> Line {
        if (index < 1 || index > 2)
            lua.argError(2, "index out of range");
        return plane.line(index - 1);
    };
    constexpr auto plane = +[](State& lua, const Plane& plane, std::size_t index1, std::size_t index2) {
        if (index1 < 1 || index1 > 2)
            lua.argError(2, "index out of range");
        if (index2 < 1 || index2 > 2)
            lua.argError(3, "index out of range");
        return plane.plane(index1 - 1, index2 - 1);
    };

    std::vector result{reg<at>("at"), reg<line>("line"), reg<plane>("plane")};

    if constexpr (v_dim == 2) {
        result.push_back(reg<&Plane::factorAt>("factorAt"));
    }
    else if constexpr (v_dim == 3) {
        result.push_back(reg<&Plane::perpendicular>("perpendicular"));
        result.push_back(reg<&Plane::normal>("normal"));
        result.push_back(reg<&Plane::height>("height"));
        result.push_back(reg<&Plane::intersectionMatrix>("intersectionMatrix"));
        result.push_back(reg<&Plane::intersectionLineFactor>("intersectionLineFactor"));
        result.push_back(reg<&Plane::intersectionPoint>("intersectionPoint"));
        result.push_back(reg<&Plane::intersectionPointViaPlane>("intersectionPointViaPlane"));
        result.push_back(reg<&Plane::intersectionLine>("intersectionLine"));
    }

    return result;
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Plane<T, v_dim>>::metatable()
{
    constexpr auto index = +[](const Plane& plane, std::variant<dang::math::Vector<T, 2>, const char*> factor) {
        return std::visit(
            dutils::Overloaded{[&](dang::math::Vector<T, 2> factor) { return std::optional(plane[factor]); },
                               [](...) { return std::optional<Vector>(); }},
            factor);
    };
    constexpr auto eq = +[](const Plane& lhs, const Plane& rhs) { return lhs == rhs; };
    constexpr auto pairs = +[](State& lua, Arg plane) {
        constexpr auto next = +[](State& lua, Arg table, Arg key) {
            auto result = table.next(std::move(key));
            return result ? VarArgs(*result) : VarArgs(lua.pushNil());
        };
        return std::tuple{wrap<next>, plane.getMetafield("indextable")};
    };

    return std::vector{reg<index>("__index"), reg<eq>("__eq"), reg<pairs>("__pairs")};
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Plane<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs args) {
        if (args.empty())
            return Plane();
        auto support = args[0].check<Vector>();
        auto direction1 = args[1].check<Vector>();
        auto direction2 = args[2].check<Vector>();
        return Plane(support, dang::math::Matrix<T, 2, v_dim>({direction1, direction2}));
    };

    auto result = lua.pushTable();

    auto result_mt = lua.pushTable();
    result_mt.setTable("__call", wrap<create>);

    result.setMetatable(std::move(result_mt));
    return result;
}

template struct ClassInfo<dang::math::Plane<float, 2>>;
template struct ClassInfo<dang::math::Plane<float, 3>>;

template struct ClassInfo<dang::math::Plane<double, 2>>;
template struct ClassInfo<dang::math::Plane<double, 3>>;

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Spat<T, v_dim>>::base_class_name = [] {
    using namespace std::literals;
    if constexpr (std::is_same_v<T, float>)
        return "Spat"s;
    else if constexpr (std::is_same_v<T, double>)
        return "DSpat"s;
    else
        static_assert(dutils::always_false_v<T>, "unsupported Spat type");
}();

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Spat<T, v_dim>>::class_name =
    ClassInfo<dang::math::Spat<T, v_dim>>::base_class_name + std::to_string(v_dim);

template <typename T, std::size_t v_dim>
const std::string ClassInfo<dang::math::Spat<T, v_dim>>::class_name_ref =
    ClassInfo<dang::math::Spat<T, v_dim>>::class_name + '&';

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Spat<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
const char* ClassInfo<dang::math::Spat<T, v_dim>>::classNameRef()
{
    return class_name_ref.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Spat<T, v_dim>>::table()
{
    constexpr auto at = +[](const Spat& spat, T x, T y, T z) { return spat[{x, y, z}]; };

    constexpr auto line = +[](State& lua, const Spat& spat, std::size_t index) -> Line {
        if (index < 1 || index > 3)
            lua.argError(2, "index out of range");
        return spat.line(index - 1);
    };
    constexpr auto plane = +[](State& lua, const Spat& spat, std::size_t index1, std::size_t index2) {
        if (index1 < 1 || index1 > 3)
            lua.argError(2, "index out of range");
        if (index2 < 1 || index2 > 3)
            lua.argError(3, "index out of range");
        return spat.plane(index1 - 1, index2 - 1);
    };
    constexpr auto spat =
        +[](State& lua, const Spat& spat, std::size_t index1, std::size_t index2, std::size_t index3) {
            if (index1 < 1 || index1 > 3)
                lua.argError(2, "index out of range");
            if (index2 < 1 || index2 > 3)
                lua.argError(3, "index out of range");
            if (index3 < 1 || index3 > 3)
                lua.argError(4, "index out of range");
            return spat.spat(index1 - 1, index2 - 1, index3 - 1);
        };

    std::vector result{reg<at>("at"), reg<line>("line"), reg<plane>("plane"), reg<spat>("spat")};

    if constexpr (v_dim == 3) {
        constexpr auto factorAt = +[](const Spat& spat, const Vector& point) { return spat.factorAt(point); };
        result.push_back(reg<factorAt>("factorAt"));
        result.push_back(reg<&Spat::tripleProduct>("tripleProduct"));
    }

    return result;
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Spat<T, v_dim>>::metatable()
{
    constexpr auto index = +[](const Spat& spat, std::variant<dang::math::Vector<T, 3>, const char*> factor) {
        return std::visit(
            dutils::Overloaded{[&](dang::math::Vector<T, 3> factor) { return std::optional(spat[factor]); },
                               [](...) { return std::optional<Vector>(); }},
            factor);
    };
    constexpr auto eq = +[](const Spat& lhs, const Spat& rhs) { return lhs == rhs; };
    constexpr auto pairs = +[](State& lua, Arg plane) {
        constexpr auto next = +[](State& lua, Arg table, Arg key) {
            auto result = table.next(std::move(key));
            return result ? VarArgs(*result) : VarArgs(lua.pushNil());
        };
        return std::tuple{wrap<next>, plane.getMetafield("indextable")};
    };

    return std::vector{reg<index>("__index"), reg<eq>("__eq"), reg<pairs>("__pairs")};
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Spat<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs args) {
        if (args.empty())
            return Spat();
        auto support = args[0].check<Vector>();
        auto direction1 = args[1].check<Vector>();
        auto direction2 = args[2].check<Vector>();
        auto direction3 = args[3].check<Vector>();
        return Spat(support, dang::math::Matrix<T, 3, v_dim>({direction1, direction2, direction3}));
    };

    auto result = lua.pushTable();

    auto result_mt = lua.pushTable();
    result_mt.setTable("__call", wrap<create>);

    result.setMetatable(std::move(result_mt));
    return result;
}

template struct ClassInfo<dang::math::Spat<float, 3>>;

template struct ClassInfo<dang::math::Spat<double, 3>>;

} // namespace dang::lua
