#pragma once

#include "exception.h"
#include <functional>
#include "LuaRef.h"
#include <memory>
#include "primitives.h"
#include "util.h"

namespace sel {

class Selector;

/*
 * Similar to an std::function but refers to a lua function
 */
template <class>
class function {};

template <typename R, typename... Args>
class function<R(Args...)> {
    friend class Selector;
private:
    LuaRef _ref;
    lua_State *_state;
    ExceptionHandler *_exception_handler;

    void _enable_exception_handler(ExceptionHandler *exception_handler) {
        _exception_handler = exception_handler;
    }
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state), _exception_handler(nullptr) {}

    R operator()(Args... args) {
        int handler_index = SetErrorHandler(_state);
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        const auto status = lua_pcall(_state, num_args, 1, handler_index);
        lua_remove(_state, handler_index);

        if(status != LUA_OK && _exception_handler) {
            _exception_handler->Handle_top_of_stack(status, _state);
        }

        R ret = detail::_pop(detail::_id<R>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};

template <typename... Args>
class function<void(Args...)> {
    friend class Selector;
private:
    LuaRef _ref;
    lua_State *_state;
    ExceptionHandler *_exception_handler;

    void _enable_exception_handler(ExceptionHandler *exception_handler) {
        _exception_handler = exception_handler;
    }
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state), _exception_handler(nullptr) {}

    void operator()(Args... args) {
        int handler_index = SetErrorHandler(_state);
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        const auto status = lua_pcall(_state, num_args, 1, handler_index);
        lua_remove(_state, handler_index);

        if(status != LUA_OK && _exception_handler) {
            _exception_handler->Handle_top_of_stack(status, _state);
        }

        lua_settop(_state, 0);
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};

// Specialization for multireturn types
template <typename... R, typename... Args>
class function<std::tuple<R...>(Args...)> {
    friend class Selector;
private:
    LuaRef _ref;
    lua_State *_state;
    ExceptionHandler *_exception_handler;

    void _enable_exception_handler(ExceptionHandler *exception_handler) {
        _exception_handler = exception_handler;
    }
public:
    function(int ref, lua_State *state) : _ref(state, ref), _state(state), _exception_handler(nullptr) {}

    std::tuple<R...> operator()(Args... args) {
        int handler_index = SetErrorHandler(_state);
        _ref.Push(_state);
        detail::_push_n(_state, args...);
        constexpr int num_args = sizeof...(Args);
        constexpr int num_ret = sizeof...(R);
        const auto status = lua_pcall(_state, num_args, num_ret, handler_index);
        lua_remove(_state, handler_index);

        if(status != LUA_OK && _exception_handler) {
            _exception_handler->Handle_top_of_stack(status, _state);
        }

        return detail::_pop_n_reset<R...>(_state);
    }

    void Push(lua_State *state) {
        _ref.Push(state);
    }
};
}
