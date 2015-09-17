#pragma once

#include <assert.h>
#include "Constants.h"
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

inline void print(std::string const& msg) {
    std::cout << msg << std::endl;
}

inline bool check(lua_State *L, int code) {
#if LUA_VERSION_NUM >= 502
    if (code == LUA_OK) {
#else
    if (code == 0) {
#endif
        return true;
    } else {
        std::cout << lua_tostring(L, -1) << std::endl;
        return false;
    }
}

using _err_handler_type = std::function<void(std::string const&)>;
inline int ErrorHandler(lua_State *L) {
    _err_handler_type *fun =
      (_err_handler_type *)lua_touserdata(L, lua_upvalueindex(1));

    const char* msg = "<not set>";
    if (!lua_isnil(L, -1)) {
        msg = lua_tostring(L, -1);
        if (!msg)
            msg = "<error object>";
    }

    // call debug.traceback
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);

    const char* stack = "<not set>";
    if (!lua_isnil(L, -1)) {
        stack = lua_tostring(L, -1);
        if (!stack)
            stack = "<error object>";
    }

    std::string errMsg(msg);
    errMsg.append(" : ");
    errMsg.append(stack);
    (*fun)(errMsg);
    return 1;
}

inline void PrepareStackAndSetErrorHandler(lua_State *L, _err_handler_type *handler) {
    assert(L != nullptr);
    assert(handler != nullptr);
    lua_settop(L, 0);

    lua_pushlightuserdata(L, (void *)(handler));
    lua_pushcclosure(L, &ErrorHandler, 1);
    assert(ErrorHandlerIndex == lua_gettop(L));
}
}
