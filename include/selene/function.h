#pragma once

#include "Constants.h"
#include <functional>
#include "LuaRef.h"
#include <memory>
#include "primitives.h"
#include "util.h"

namespace sel {
/*
 * Similar to an std::function but refers to a lua function
 */
template <class>
class function {};

template <typename R, typename... Args>
class function<R(Args...)> {
private:
    LuaRef _ref;
    lua_State *_state;
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state) {}

    R operator()(Args... args) {
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        lua_pcall(_state, num_args, 1, ErrorHandlerIndex);
        R ret = detail::_pop(detail::_id<R>{}, _state);
        lua_settop(_state, ErrorHandlerIndex);
        return ret;
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};

template <typename... Args>
class function<void(Args...)> {
private:
    LuaRef _ref;
    lua_State *_state;
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state) {}

    void operator()(Args... args) {
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        lua_pcall(_state, num_args, 1, ErrorHandlerIndex);
        lua_settop(_state, ErrorHandlerIndex);
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};

// Specialization for multireturn types
template <typename... R, typename... Args>
class function<std::tuple<R...>(Args...)> {
private:
    LuaRef _ref;
    lua_State *_state;
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state) {}

    std::tuple<R...> operator()(Args... args) {
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        constexpr int num_ret = sizeof...(R);
        lua_pcall(_state, num_args, num_ret, ErrorHandlerIndex);
        return detail::_pop_n_reset<R...>(_state);
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};
}
