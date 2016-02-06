#pragma once

#include <memory>
#include <vector>
#include "primitives.h"
#include "ResourceHandler.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace sel {
class LuaRef {
private:
	lua_State *_state;
    int _ref;
public:
    LuaRef(lua_State *state, int ref)
        : _ref(ref) 
		, _state(state)
	{}

    LuaRef(lua_State *state)
        : LuaRef(state, LUA_REFNIL)
        {}

	LuaRef(const LuaRef &other)
		: _state(other._state)
		, _ref(other._ref)
	{
		if (_state && _ref != LUA_REFNIL)
		{
			other.Push(_state);
			_ref = luaL_ref(_state, LUA_REGISTRYINDEX);
		}
	}

	~LuaRef()
	{
		if(_state && _ref != LUA_REFNIL)
			luaL_unref(_state, LUA_REGISTRYINDEX, _ref);
	}

    void Push(lua_State *state) const {
        lua_rawgeti(state, LUA_REGISTRYINDEX, _ref);
    }
};

template <typename T>
LuaRef make_Ref(lua_State * state, T&& t) {
    detail::_push(state, std::forward<T>(t));
    return LuaRef(state, luaL_ref(state, LUA_REGISTRYINDEX));
}

namespace detail {
    inline void append_ref_recursive(lua_State *, std::vector<LuaRef> &) {}

    template <typename Head, typename... Tail>
    void append_ref_recursive(lua_State * state, std::vector<LuaRef> & refs, Head&& head, Tail&&... tail) {
        refs.push_back(make_Ref(state, std::forward<Head>(head)));

        append_ref_recursive(state, refs, std::forward<Tail>(tail)...);
    }
}

template <typename... Args>
std::vector<LuaRef> make_Refs(lua_State * state, Args&&... args) {
    std::vector<LuaRef> refs;
    refs.reserve(sizeof...(Args));

    detail::append_ref_recursive(state, refs, std::forward<Args>(args)...);
    return refs;
}
}
