#pragma once

#include <memory>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace sel {
namespace detail {
class LuaRefDeleter {
private:
    lua_State *_state;
public:
    LuaRefDeleter(lua_State *state) : _state{state} {}
    void operator()(int *ref) const {
        luaL_unref(_state, LUA_REGISTRYINDEX, *ref);
        delete ref;
    }
};
}
class LuaRef {
private:
    std::shared_ptr<int> _ref;
public:
    LuaRef(lua_State *state, int ref)
        : _ref(new int{ref}, detail::LuaRefDeleter{state}) {}

    void Push(lua_State *state) {
        lua_rawgeti(state, LUA_REGISTRYINDEX, *_ref);
    }
};
}
