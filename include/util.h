#pragma once

#include <iostream>
#include <tuple>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace luna {
std::ostream &operator<<(std::ostream &os, lua_State *l);

inline bool check(lua_State *L, int code) {
    if (code == LUA_OK) {
        return true;
    } else {
        std::cout << lua_tostring(L, -1) << std::endl;
        return false;
    }
}

namespace detail {
template <std::size_t... Is>
struct _indices {};

template <std::size_t N, std::size_t... Is>
struct _indices_builder : _indices_builder<N-1, N-1, Is...> {};

template <std::size_t... Is>
struct _indices_builder<0, Is...> : _indices<Is...> {
    using type = _indices<Is...>;
};

template <typename T> T _get(lua_State *l, const int index);

template <typename T> T _check_get(lua_State *l, const int index);

inline void _push(lua_State *l) {} // Necessary in the case no arguments are passed
void _push(lua_State *l, bool &&value);
void _push(lua_State *l, int &&value);
void _push(lua_State *l, unsigned int &&value);
void _push(lua_State *l, float &&value);
void _push(lua_State *l, double &&value);
void _push(lua_State *l, std::string &&value);

template <typename T>
void _set(lua_State *l, T &&value, const int index) {
    _push(l, std::forward<T>(value));
    lua_replace(l, index);
}

inline void _push_n(lua_State *) {}

template <typename T, typename... Rest>
void _push_n(lua_State *l, T value, Rest... rest) {
    _push(l, std::forward<T>(value));
    _push_n(l, rest...);
}

template <typename... T, std::size_t... N>
void _push_dispatcher(lua_State *l,
                      std::tuple<T...> &&values,
                      _indices<N...>) {
    _push_n(l, std::get<N>(values)...);
}

template <typename... T>
void _push(lua_State *l, std::tuple<T...> &&values) {
    constexpr int num_values = sizeof...(T);
    lua_settop(l, num_values);
    _push_dispatcher(l,
                     std::forward<std::tuple<T...>>(values),
                     typename _indices_builder<num_values>::type());
}

// Worker type-trait struct to _pop_n
// Popping multiple elements returns a tuple
template <std::size_t, typename... Ts> // First template argument denotes
                                       // the sizeof(Ts...)
struct _pop_n_impl {
    using type =  std::tuple<Ts...>;

    template <std::size_t... N>
    static type worker(lua_State *l,
                       const int index,
                       _indices<N...>) {
        return std::make_tuple(_get<Ts>(l, N + 1)...);
    }

    static type apply(lua_State *l) {
        constexpr std::size_t num_args = sizeof...(Ts);
        auto ret = worker(l, 1, typename _indices_builder<num_args>::type());
        lua_pop(l, num_args);
        return ret;
    }
};

// Popping nothing returns void
template <typename... Ts>
struct _pop_n_impl<0, Ts...> {
    using type = void;
    static type apply(lua_State *) {}
};

// Popping one element returns an unboxed value
template <typename T>
struct _pop_n_impl<1, T> {
    using type = T;
    static type apply(lua_State *l) {
        T ret = _get<T>(l, -1);
        lua_pop(l, 1);
        return ret;
    }
};

template <typename... T>
typename _pop_n_impl<sizeof...(T), T...>::type _pop_n(lua_State *l) {
    return _pop_n_impl<sizeof...(T), T...>::apply(l);
}
}
}
