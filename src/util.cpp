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
template <>
bool _get<bool>(lua_State *l, const int index) {
    return lua_toboolean(l, index);
}

template <>
int _get<int>(lua_State *l, const int index) {
    return lua_tointeger(l, index);
}

template <>
unsigned int _get<unsigned int>(lua_State *l, const int index) {
    return lua_tounsigned(l, index);
}

template <>
float _get<float>(lua_State *l, const int index) {
    return lua_tonumber(l, index);
}

template <>
double _get<double>(lua_State *l, const int index) {
    return lua_tonumber(l, index);
}

template <>
std::string _get<std::string>(lua_State *l, const int index) {
    size_t size;
    const char *buff = lua_tolstring(l, index, &size);
    return std::string{buff, size};
}

template <>
int _check_get<int>(lua_State *l, const int index) {
    return luaL_checkint(l, index);
};

template <>
unsigned int _check_get<unsigned int>(lua_State *l, const int index) {
    return luaL_checkunsigned(l, index);
}

template <>
float _check_get<float>(lua_State *l, const int index) {
    return luaL_checknumber(l, index);
}

template <>
double _check_get<double>(lua_State *l, const int index) {
    return luaL_checknumber(l, index);
}

template <>
bool _check_get<bool>(lua_State *l, const int index) {
    return lua_toboolean(l, index);
}

template <>
std::string _check_get<std::string>(lua_State *l, const int index) {
    size_t size;
    const char *buff = luaL_checklstring(l, index, &size);
    return std::string{buff, size};
}

void _push(lua_State *l, bool &&b) {
    lua_pushboolean(l, b);
}

void _push(lua_State *l, int &&i) {
    lua_pushinteger(l, i);
}

void _push(lua_State *l, unsigned int &&u) {
    lua_pushunsigned(l, u);
}

void _push(lua_State *l, float &&f) {
    lua_pushnumber(l, f);
}

void _push(lua_State *l, double &&d) {
    lua_pushnumber(l, d);
}

void _push(lua_State *l, std::string &&s) {
    lua_pushlstring(l, s.c_str(), s.size());
}
}
}
