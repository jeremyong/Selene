#pragma once

#include <iostream>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace sel {
inline std::ostream &operator<<(std::ostream &os, lua_State *l) {
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

inline void _print() {
    std::cout << std::endl;
}

template <typename T, typename... Ts>
inline void _print(T arg, Ts... args) {
    std::cout << arg << ", ";
    _print(args...);
}

inline bool check(lua_State *L, int code) {
    if (code == LUA_OK) {
        return true;
    } else {
        std::cout << lua_tostring(L, -1) << std::endl;
        return false;
    }
}
}
