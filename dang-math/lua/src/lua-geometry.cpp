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
const char* ClassInfo<dang::math::Line<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Line<T, v_dim>>::table()
{
    constexpr auto support = +[](const Line& line) { return line.support; };
    constexpr auto setSupport = +[](Line& line, const Point& support) { line.support = support; };
    constexpr auto direction = +[](const Line& line) { return line.direction(); };
    constexpr auto setDirection = +[](Line& line, const Direction& direction) { line.direction() = direction; };

    std::vector result{reg<support>("support"),
                       reg<setSupport>("setSupport"),
                       reg<direction>("direction"),
                       reg<setDirection>("setDirection"),
                       reg<&Line::head, Line>("head"),
                       reg<&Line::setHead, Line>("setHead"),
                       reg<&Line::tail, Line>("tail"),
                       reg<&Line::setTail, Line>("setTail"),
                       reg<&Line::length, Line>("length"),
                       reg<&Line::mirror, Line>("mirror")};

    if constexpr (v_dim == 2) {
        result.push_back(reg<&Line::closestFactorTo, Line>("closestFactorTo"));
        result.push_back(reg<&Line::closestPointTo, Line>("closestPointTo"));
        result.push_back(reg<&Line::heightTo>("heightTo"));
        result.push_back(reg<&Line::distanceTo>("distanceTo"));
        result.push_back(reg<&Line::sideOf>("sideOf"));
        result.push_back(reg<&Line::intersectionMatrix>("intersectionMatrix"));
        result.push_back(reg<&Line::intersectionFactor>("intersectionFactor"));
        result.push_back(reg<&Line::intersectionFactors>("intersectionFactors"));
        result.push_back(reg<&Line::intersectionPoint>("intersectionPoint"));
    }
    else if constexpr (v_dim == 3) {
        constexpr auto closestFactorTo = +[](const Line& line, std::variant<Point, Line> target) {
            return std::visit([&](const auto& target) { return line.closestFactorTo(target); }, target);
        };
        constexpr auto closestPointTo = +[](const Line& line, std::variant<Point, Line> target) {
            return std::visit([&](const auto& target) { return line.closestPointTo(target); }, target);
        };

        result.push_back(reg<&Line::distanceTo>("distanceTo"));
        result.push_back(reg<closestFactorTo>("closestFactorTo"));
        result.push_back(reg<closestPointTo>("closestPointTo"));
    }

    return result;
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Line<T, v_dim>>::metatable()
{
    constexpr auto index = +[](const Line& line, std::variant<T, const char*> factor) {
        return std::visit(dutils::Overloaded{[&](T factor) { return std::optional(line[factor]); },
                                             [](...) { return std::optional<Point>(); }},
                          factor);
    };
    constexpr auto eq = +[](const Line& lhs, const Line& rhs) { return lhs == rhs; };

    return std::vector{reg<index>("__index"), reg<eq>("__eq"), reg<indextable_pairs>("__pairs")};
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Line<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs args) {
        if (args.empty())
            return Line();
        auto support = args[0].check<Point>();
        auto direction = args[1].check<Direction>();
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
const char* ClassInfo<dang::math::Plane<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Plane<T, v_dim>>::table()
{
    constexpr auto at = +[](const Plane& plane, Factor x, Factor y) { return plane[{x, y}]; };
    constexpr auto line = +[](State& lua, const Plane& plane, std::size_t index) {
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
    constexpr auto support = +[](const Plane& plane) { return plane.support; };
    constexpr auto setSupport = +[](Plane& plane, const Point& support) { plane.support = support; };
    constexpr auto directions = +[](const Plane& plane) { return plane.directions; };
    constexpr auto setDirections = +[](Plane& plane, const Directions& directions) { plane.directions = directions; };
    constexpr auto direction = +[](State& lua, const Plane& plane, std::size_t index) {
        if (index < 1 || index > 2)
            lua.argError(2, "index out of range");
        return plane.directions[index - 1];
    };
    constexpr auto setDirection = +[](State& lua, Plane& plane, std::size_t index, const Direction& direction) {
        if (index < 1 || index > 2)
            lua.argError(2, "index out of range");
        plane.directions[index - 1] = direction;
    };
    constexpr auto quadPoint = +[](State& lua, const Plane& plane, std::size_t index) {
        if (index < 1 || index > 4)
            lua.argError(2, "index out of range");
        return plane.quadPoint(index - 1);
    };
    constexpr auto trianglePoint = +[](State& lua, const Plane& plane, std::size_t index) {
        if (index < 1 || index > 3)
            lua.argError(2, "index out of range");
        return plane.trianglePoint(index - 1);
    };
    constexpr auto innerRadians = +[](State& lua, const Plane& plane, std::size_t index) {
        if (index < 1 || index > 3)
            lua.argError(2, "index out of range");
        return plane.innerRadians(index - 1);
    };
    constexpr auto innerDegrees = +[](State& lua, const Plane& plane, std::size_t index) {
        if (index < 1 || index > 3)
            lua.argError(2, "index out of range");
        return plane.innerDegrees(index - 1);
    };

    std::vector result{reg<at>("at"),
                       reg<line>("line"),
                       reg<plane>("plane"),
                       reg<support>("support"),
                       reg<setSupport>("setSupport"),
                       reg<directions>("directions"),
                       reg<setDirections>("setDirections"),
                       reg<direction>("direction"),
                       reg<setDirection>("setDirection"),
                       reg<&Plane::area, Plane>("area"),
                       reg<&Plane::closestFactorTo, Plane>("closestFactorTo"),
                       reg<&Plane::closestPointTo, Plane>("closestPointTo"),
                       reg<quadPoint>("quadPoint"),
                       reg<trianglePoint>("trianglePoint"),
                       reg<innerRadians>("innerRadians"),
                       reg<innerDegrees>("innerDegrees")};

    if constexpr (v_dim == 2) {
        result.push_back(reg<&Plane::factorAt>("factorAt"));
    }
    else if constexpr (v_dim == 3) {
        constexpr auto radiansTo = +[](const Plane& plane, std::variant<Point, Plane> target) {
            return std::visit([&](const auto& target) { return plane.radiansTo(target); }, target);
        };
        constexpr auto degreesTo = +[](const Plane& plane, std::variant<Point, Plane> target) {
            return std::visit([&](const auto& target) { return plane.degreesTo(target); }, target);
        };

        result.push_back(reg<&Plane::perpendicular>("perpendicular"));
        result.push_back(reg<&Plane::perpendicularLine>("perpendicularLine"));
        result.push_back(reg<&Plane::normal>("normal"));
        result.push_back(reg<&Plane::normalLine>("normalLine"));
        result.push_back(reg<&Plane::heightTo>("heightTo"));
        result.push_back(reg<&Plane::distanceTo>("distanceTo"));
        result.push_back(reg<&Plane::sideOf>("sideOf"));
        result.push_back(reg<&Plane::intersectionMatrix>("intersectionMatrix"));
        result.push_back(reg<&Plane::intersectionFactors>("intersectionFactors"));
        result.push_back(reg<&Plane::intersectionLineFactor>("intersectionLineFactor"));
        result.push_back(reg<&Plane::intersectionPoint>("intersectionPoint"));
        result.push_back(reg<&Plane::intersectionPointViaPlane>("intersectionPointViaPlane"));
        result.push_back(reg<&Plane::intersectionLine>("intersectionLine"));
        result.push_back(reg<&Plane::cosAngleToPerpendicular>("cosAngleToPerpendicular"));
        result.push_back(reg<&Plane::radiansToPerpendicular>("radiansToPerpendicular"));
        result.push_back(reg<&Plane::degreesToPerpendicular>("degreesToPerpendicular"));
        result.push_back(reg<radiansTo>("radiansTo"));
        result.push_back(reg<degreesTo>("degreesTo"));
        result.push_back(reg<&Plane::cosAngleTo>("cosAngleTo"));
        result.push_back(reg<&Plane::mirror>("mirror"));
    }

    return result;
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Plane<T, v_dim>>::metatable()
{
    constexpr auto index = +[](const Plane& plane, std::variant<Factors, const char*> factor) {
        return std::visit(dutils::Overloaded{[&](const Factors& factor) { return std::optional(plane[factor]); },
                                             [](...) { return std::optional<Point>(); }},
                          factor);
    };
    constexpr auto eq = +[](const Plane& lhs, const Plane& rhs) { return lhs == rhs; };

    return std::vector{reg<index>("__index"), reg<eq>("__eq"), reg<indextable_pairs>("__pairs")};
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Plane<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs args) {
        if (args.empty())
            return Plane();
        auto support = args[0].check<Point>();
        auto direction1 = args[1].check<Direction>();
        auto direction2 = args[2].check<Direction>();
        return Plane(support, Directions({direction1, direction2}));
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
const char* ClassInfo<dang::math::Spat<T, v_dim>>::className()
{
    return class_name.c_str();
}

template <typename T, std::size_t v_dim>
std::vector<luaL_Reg> ClassInfo<dang::math::Spat<T, v_dim>>::table()
{
    constexpr auto at = +[](const Spat& spat, Factor x, Factor y, Factor z) { return spat[{x, y, z}]; };
    constexpr auto line = +[](State& lua, const Spat& spat, std::size_t index) {
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
    constexpr auto support = +[](const Spat& spat) { return spat.support; };
    constexpr auto setSupport = +[](Spat& spat, const Point& support) { spat.support = support; };
    constexpr auto directions = +[](const Spat& spat) { return spat.directions; };
    constexpr auto setDirections = +[](Spat& spat, const Directions& directions) { spat.directions = directions; };
    constexpr auto direction = +[](State& lua, const Spat& spat, std::size_t index) {
        if (index < 1 || index > 3)
            lua.argError(2, "index out of range");
        return spat.directions[index - 1];
    };
    constexpr auto setDirection = +[](State& lua, Spat& spat, std::size_t index, const Direction& direction) {
        if (index < 1 || index > 3)
            lua.argError(2, "index out of range");
        spat.directions[index - 1] = direction;
    };

    std::vector result{reg<at>("at"),
                       reg<line>("line"),
                       reg<plane>("plane"),
                       reg<spat>("spat"),
                       reg<support>("support"),
                       reg<setSupport>("setSupport"),
                       reg<directions>("directions"),
                       reg<setDirections>("setDirections"),
                       reg<direction>("direction"),
                       reg<setDirection>("setDirection")};

    if constexpr (v_dim == 3) {
        result.push_back(reg<&Spat::factorAt, Spat>("factorAt"));
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
                               [](...) { return std::optional<Point>(); }},
            factor);
    };
    constexpr auto eq = +[](const Spat& lhs, const Spat& rhs) { return lhs == rhs; };

    return std::vector{reg<index>("__index"), reg<eq>("__eq"), reg<indextable_pairs>("__pairs")};
}

template <typename T, std::size_t v_dim>
Arg ClassInfo<dang::math::Spat<T, v_dim>>::require(State& lua)
{
    constexpr auto create = +[](State& lua, Arg, VarArgs args) {
        if (args.empty())
            return Spat();
        auto support = args[0].check<Point>();
        auto direction1 = args[1].check<Direction>();
        auto direction2 = args[2].check<Direction>();
        auto direction3 = args[3].check<Direction>();
        return Spat(support, Directions({direction1, direction2, direction3}));
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
