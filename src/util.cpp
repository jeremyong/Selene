#include "util.h"

namespace luna {
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
    return lua_tostring(l, index);
}

template <>
int _check_get<int>(lua_State *l) {
    return luaL_checkint(l, 1);
};

template <>
unsigned int _check_get<unsigned int>(lua_State *l) {
    return luaL_checkunsigned(l, 1);
}

template <>
float _check_get<float>(lua_State *l) {
    return luaL_checknumber(l, 1);
}

template <>
double _check_get<double>(lua_State *l) {
    return luaL_checknumber(l, 1);
}

template <>
bool _check_get<bool>(lua_State *l) {
    return lua_toboolean(l, 1);
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
