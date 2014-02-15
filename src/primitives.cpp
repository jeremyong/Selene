#include "util.h"

namespace sel {
std::ostream &operator<<(std::ostream &os, lua_State *l) {
    int top = lua_gettop(l);
    for (int i = 1; i <= top; ++i) {
        int t = lua_type(l, i);
        switch(t) {
        case LUA_TSTRING:
            os << lua_tostring(l, i);
            break;
        case LUA_TBOOLEAN:
            os << (lua_toboolean(l, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            os << lua_tonumber(l, i);
            break;
        default:
            os << lua_typename(l, t);
            break;
        }
        os << " ";
    }
    return os;
}

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
