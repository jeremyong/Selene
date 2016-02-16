#pragma once

#include <string>
#include <type_traits>
#include "types.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

/* The purpose of this header is to handle pushing and retrieving
 * primitives from the stack
 */

namespace sel {

namespace detail {

using _lua_check_get = void (*)(lua_State *l, int index);
// Throw this on conversion errors to prevent long jumps caused in Lua from
// bypassing destructors. The outermost function can then call checked_get(index)
// in a context where a long jump is safe.
// This way we let Lua generate the error message and use proper stack
// unwinding.
struct GetParameterFromLuaTypeError {
    _lua_check_get checked_get;
    int index;
};

template<>
struct type_t<int> {

    int _get(_id<int>, lua_State *l, const int index) {
        return static_cast<int>(lua_tointeger(l, index));
    }

    int _check_get(_id<int>, lua_State *l, const int index) {
#if LUA_VERSION_NUM >= 502
        int isNum = 0;
        auto res = static_cast<int>(lua_tointegerx(l, index, &isNum));
        if(!isNum){
            throw GetParameterFromLuaTypeError{
#if LUA_VERSION_NUM >= 503
                [](lua_State *l, int index){luaL_checkinteger(l, index);},
#else
                    [](lua_State *l, int index){luaL_checkint(l, index);},
#endif
                    index
            };
        }
        return res;
#else
#error "Not supported for Lua versions <5.2"
#endif
    }

    void _push(lua_State *l, int i) {
        lua_pushinteger(l, i);
    }

};

template<>
struct type_t<unsigned int> {

    unsigned int _get(_id<unsigned int>, lua_State *l, const int index) {
#if LUA_VERSION_NUM >= 502 && LUA_VERSION_NUM < 503
        return lua_tounsigned(l, index);
#else
        return static_cast<unsigned>(lua_tointeger(l, index));
#endif
    }

    unsigned int _check_get(_id<unsigned int>, lua_State *l, const int index) {
        int isNum = 0;
#if LUA_VERSION_NUM >= 503
        auto res = static_cast<unsigned>(lua_tointegerx(l, index, &isNum));
        if(!isNum) {
            throw GetParameterFromLuaTypeError{
                [](lua_State *l, int index){luaL_checkinteger(l, index);},
                    index
            };
        }
#elif LUA_VERSION_NUM >= 502
        auto res = static_cast<unsigned>(lua_tounsignedx(l, index, &isNum));
        if(!isNum) {
            throw GetParameterFromLuaTypeError{
                [](lua_State *l, int index){luaL_checkunsigned(l, index);},
                    index
            };
        }
#else
#error "Not supported for Lua versions <5.2"
#endif
        return res;
    }

    void _push(lua_State *l, unsigned int u) {
#if LUA_VERSION_NUM >= 503
        lua_pushinteger(l, (lua_Integer)u);
#elif LUA_VERSION_NUM >= 502
        lua_pushunsigned(l, u);
#else
        lua_pushinteger(l, static_cast<int>(u));
#endif
    }

};

template<>
struct type_t<bool> {

    bool _get(_id<bool>, lua_State *l, const int index) {
        return lua_toboolean(l, index) != 0;
    }

    bool _check_get(_id<bool> id, lua_State *l, const int index) {
        return this->_get(id, l, index);
    }

    void _push(lua_State *l, bool b) {
        lua_pushboolean(l, b);
    }

};

template<>
struct type_t<lua_Number> {

    lua_Number _get(_id<lua_Number>, lua_State *l, const int index) {
        return lua_tonumber(l, index);
    }

    lua_Number _check_get(_id<lua_Number>, lua_State *l, const int index) {
        int isNum = 0;
        auto res = lua_tonumberx(l, index, &isNum);
        if(!isNum){
            throw GetParameterFromLuaTypeError{
                [](lua_State *l, int index){luaL_checknumber(l, index);},
                    index
            };
        }
        return res;
    }

    inline void _push(lua_State *l, lua_Number f) {
        lua_pushnumber(l, f);
    }

};

template<>
struct type_t<std::string> {

    std::string _get(_id<std::string>, lua_State *l, const int index) {
        size_t size;
        const char *buff = lua_tolstring(l, index, &size);
        return std::string{buff, size};
    }

    std::string _check_get(_id<std::string>, lua_State *l, const int index) {
        size_t size = 0;
        char const * buff = lua_tolstring(l, index, &size);
        if(buff == nullptr) {
            throw GetParameterFromLuaTypeError{
                [](lua_State *l, int index){luaL_checkstring(l, index);},
                    index
            };
        }
        return std::string{buff, size};
    }

    void _push(lua_State *l, const std::string &s) {
        lua_pushlstring(l, s.c_str(), s.size());
    }

};

// It should be possible to add referenceable_tag and implement _get and _check_get.
template<>
struct type_t<const char *> {

    void _push(lua_State *l, const char *s) {
        lua_pushstring(l, s);
    }

};

}
}
