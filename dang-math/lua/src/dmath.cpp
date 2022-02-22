#include "dang-math/dmath.h"

#include "dang-lua/convert/String.h"
#include "dang-math/lua-geometry.h"
#include "dang-math/lua-vector-matrix.h"

namespace dlua = dang::lua;
namespace dmath = dang::math;

namespace {

// TODO: C++20 use a templated lambda and just capture table.
// TODO: Additionally, replace table.state() with lua, which can just be captured as well.
template <typename T>
void add(dlua::Arg table)
{
    table.setTable(dlua::ClassInfo<T>::getCheckTypename().c_str(), table.state().pushRequire<T>());
}

auto open(dlua::StateRef& lua)
{
    lua.checkVersion();

    auto table = lua.pushEmptyTable();

    add<dmath::Vector<float, 2>>(table);
    add<dmath::Vector<float, 3>>(table);
    add<dmath::Vector<float, 4>>(table);

    add<dmath::Vector<double, 2>>(table);
    add<dmath::Vector<double, 3>>(table);
    add<dmath::Vector<double, 4>>(table);

    add<dmath::Vector<int, 2>>(table);
    add<dmath::Vector<int, 3>>(table);
    add<dmath::Vector<int, 4>>(table);

    add<dmath::Vector<unsigned, 2>>(table);
    add<dmath::Vector<unsigned, 3>>(table);
    add<dmath::Vector<unsigned, 4>>(table);

    add<dmath::Vector<std::size_t, 2>>(table);
    add<dmath::Vector<std::size_t, 3>>(table);
    add<dmath::Vector<std::size_t, 4>>(table);

    add<dmath::Vector<bool, 2>>(table);
    add<dmath::Vector<bool, 3>>(table);
    add<dmath::Vector<bool, 4>>(table);

    add<dmath::Matrix<float, 2, 2>>(table);
    add<dmath::Matrix<float, 2, 3>>(table);
    add<dmath::Matrix<float, 2, 4>>(table);
    add<dmath::Matrix<float, 3, 2>>(table);
    add<dmath::Matrix<float, 3, 3>>(table);
    add<dmath::Matrix<float, 3, 4>>(table);
    add<dmath::Matrix<float, 4, 2>>(table);
    add<dmath::Matrix<float, 4, 3>>(table);
    add<dmath::Matrix<float, 4, 4>>(table);

    add<dmath::Matrix<double, 2, 2>>(table);
    add<dmath::Matrix<double, 2, 3>>(table);
    add<dmath::Matrix<double, 2, 4>>(table);
    add<dmath::Matrix<double, 3, 2>>(table);
    add<dmath::Matrix<double, 3, 3>>(table);
    add<dmath::Matrix<double, 3, 4>>(table);
    add<dmath::Matrix<double, 4, 2>>(table);
    add<dmath::Matrix<double, 4, 3>>(table);
    add<dmath::Matrix<double, 4, 4>>(table);

    add<dmath::Line<float, 2>>(table);
    add<dmath::Line<float, 3>>(table);
    add<dmath::Line<double, 2>>(table);
    add<dmath::Line<double, 3>>(table);

    add<dmath::Plane<float, 2>>(table);
    add<dmath::Plane<float, 3>>(table);
    add<dmath::Plane<double, 2>>(table);
    add<dmath::Plane<double, 3>>(table);

    add<dmath::Spat<float, 3>>(table);
    add<dmath::Spat<double, 3>>(table);

    return table;
}

} // namespace

extern "C" int luaopen_dmath(lua_State* L) { return dlua::wrap<open>(L); }
