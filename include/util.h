#pragma once

#include <iostream>
#include "function.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace sel {
std::ostream &operator<<(std::ostream &os, lua_State *l);

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
