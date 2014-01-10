#pragma once

#include <iostream>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace luab {
inline bool check(lua_State *L, int code) {
    if (code == LUA_OK) {
        return true;
    } else {
        std::cout << lua_tostring(L, -1) << std::endl;
        return false;
    }
}
}
