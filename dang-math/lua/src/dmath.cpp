#include "dang-math/lua-geometry.h"
#include "dang-math/lua-vector-matrix.h"

template <typename T>
void add(dang::lua::State& lua, dang::lua::Arg table)
{
    auto name = lua(dang::lua::ClassInfo<T>::className());
    auto lib = table.state().pushRequire<T>(false);
    table.rawSetTable(std::move(name), std::move(lib));
}

extern "C" __declspec(dllexport) int luaopen_dmath(lua_State* L)
{
    dang::lua::State lua(L);
    lua.checkVersion();
    auto table = lua.pushTable();

    add<dang::math::Vector<float, 2>>(lua, table);
    add<dang::math::Vector<float, 3>>(lua, table);
    add<dang::math::Vector<float, 4>>(lua, table);

    add<dang::math::Vector<double, 2>>(lua, table);
    add<dang::math::Vector<double, 3>>(lua, table);
    add<dang::math::Vector<double, 4>>(lua, table);

    add<dang::math::Vector<int, 2>>(lua, table);
    add<dang::math::Vector<int, 3>>(lua, table);
    add<dang::math::Vector<int, 4>>(lua, table);

    add<dang::math::Vector<unsigned, 2>>(lua, table);
    add<dang::math::Vector<unsigned, 3>>(lua, table);
    add<dang::math::Vector<unsigned, 4>>(lua, table);

    add<dang::math::Vector<std::size_t, 2>>(lua, table);
    add<dang::math::Vector<std::size_t, 3>>(lua, table);
    add<dang::math::Vector<std::size_t, 4>>(lua, table);

    add<dang::math::Vector<bool, 2>>(lua, table);
    add<dang::math::Vector<bool, 3>>(lua, table);
    add<dang::math::Vector<bool, 4>>(lua, table);

    add<dang::math::Matrix<float, 2, 2>>(lua, table);
    add<dang::math::Matrix<float, 2, 3>>(lua, table);
    add<dang::math::Matrix<float, 2, 4>>(lua, table);
    add<dang::math::Matrix<float, 3, 2>>(lua, table);
    add<dang::math::Matrix<float, 3, 3>>(lua, table);
    add<dang::math::Matrix<float, 3, 4>>(lua, table);
    add<dang::math::Matrix<float, 4, 2>>(lua, table);
    add<dang::math::Matrix<float, 4, 3>>(lua, table);
    add<dang::math::Matrix<float, 4, 4>>(lua, table);

    add<dang::math::Matrix<double, 2, 2>>(lua, table);
    add<dang::math::Matrix<double, 2, 3>>(lua, table);
    add<dang::math::Matrix<double, 2, 4>>(lua, table);
    add<dang::math::Matrix<double, 3, 2>>(lua, table);
    add<dang::math::Matrix<double, 3, 3>>(lua, table);
    add<dang::math::Matrix<double, 3, 4>>(lua, table);
    add<dang::math::Matrix<double, 4, 2>>(lua, table);
    add<dang::math::Matrix<double, 4, 3>>(lua, table);
    add<dang::math::Matrix<double, 4, 4>>(lua, table);

    add<dang::math::Line<float, 2>>(lua, table);
    add<dang::math::Line<float, 3>>(lua, table);
    add<dang::math::Line<double, 2>>(lua, table);
    add<dang::math::Line<double, 3>>(lua, table);

    add<dang::math::Plane<float, 2>>(lua, table);
    add<dang::math::Plane<float, 3>>(lua, table);
    add<dang::math::Plane<double, 2>>(lua, table);
    add<dang::math::Plane<double, 3>>(lua, table);

    add<dang::math::Spat<float, 3>>(lua, table);
    add<dang::math::Spat<double, 3>>(lua, table);

    return 1;
}
