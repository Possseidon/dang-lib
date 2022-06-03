#include "dang-math-lua/dmath/export.h"

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

extern "C" DMATH_EXPORT int luaopen_dmath(lua_State* L);
