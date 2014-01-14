#pragma once

#include <iostream>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace luna {
inline bool check(lua_State *L, int code) {
    if (code == LUA_OK) {
        return true;
    } else {
        std::cout << lua_tostring(L, -1) << std::endl;
        return false;
    }
}

namespace detail {
template <typename T> T _get(lua_State *l, const int index);

template <typename T> T _check_get(lua_State *l);

inline void _push(lua_State *l) {} // Necessary in the case no arguments are passed
void _push(lua_State *l, bool &&value);
void _push(lua_State *l, int &&value);
void _push(lua_State *l, unsigned int &&value);
void _push(lua_State *l, float &&value);
void _push(lua_State *l, double &&value);
void _push(lua_State *l, std::string &&value);
}
}
