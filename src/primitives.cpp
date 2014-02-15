#include "primitives.h"

namespace sel {
namespace detail {
void _push(lua_State *l, bool b) {
    lua_pushboolean(l, b);
}

void _push(lua_State *l, int i) {
    lua_pushinteger(l, i);
}

void _push(lua_State *l, unsigned int u) {
    lua_pushunsigned(l, u);
}

void _push(lua_State *l, lua_Number f) {
    lua_pushnumber(l, f);
}

void _push(lua_State *l, const std::string &s) {
    lua_pushlstring(l, s.c_str(), s.size());
}

void _push(lua_State *l, const char *s) {
    lua_pushstring(l, s);
}
}
}
