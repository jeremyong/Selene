#pragma once

#include <iostream>
#include <tuple>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace sel {
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

template <typename T>
struct _arity {
    static constexpr int value = 1;
};

template <typename... Vs>
struct _arity<std::tuple<Vs...>> {
    static constexpr int value = sizeof...(Vs);
};

template <>
struct _arity<void> {
    static constexpr int value = 0;
};

template <std::size_t... Is>
struct _indices {};

template <std::size_t N, std::size_t... Is>
struct _indices_builder : _indices_builder<N-1, N-1, Is...> {};

template <std::size_t... Is>
struct _indices_builder<0, Is...> {
    using type = _indices<Is...>;
};

template <typename T> struct _id {};


inline bool _get(_id<bool>, lua_State *l, const int index) {
    return lua_toboolean(l, index);
}

inline int _get(_id<int>, lua_State *l, const int index) {
    return lua_tointeger(l, index);
}

inline unsigned int _get(_id<unsigned int>, lua_State *l, const int index) {
    return lua_tounsigned(l, index);
}

inline lua_Number _get(_id<lua_Number>, lua_State *l, const int index) {
    return lua_tonumber(l, index);
}

inline std::string _get(_id<std::string>, lua_State *l, const int index) {
    size_t size;
    const char *buff = lua_tolstring(l, index, &size);
    return std::string{buff, size};
}

template <typename T> T _check_get(lua_State *l, const int index);

inline void _push(lua_State *l) {} // Necessary in the case no arguments are passed
void _push(lua_State *l, bool value);
void _push(lua_State *l, int value);
void _push(lua_State *l, unsigned int value);
void _push(lua_State *l, lua_Number value);
void _push(lua_State *l, const std::string &value);

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
                      const std::tuple<T...> &values,
                      _indices<N...>) {
    _push_n(l, std::get<N>(values)...);
}

inline void _push(lua_State *l, std::tuple<>) {}

template <typename... T>
void _push(lua_State *l, const std::tuple<T...> &values) {
    constexpr int num_values = sizeof...(T);
    _push_dispatcher(l, values,
                     typename _indices_builder<num_values>::type());
}

// Worker type-trait struct to _pop_n
// Popping multiple elements returns a tuple
template <std::size_t S, typename... Ts> // First template argument denotes
                                         // the sizeof(Ts...)
struct _pop_n_impl {
    using type =  std::tuple<Ts...>;

    template <std::size_t... N>
    static type worker(lua_State *l,
                       _indices<N...>) {
        return std::make_tuple(_get(_id<Ts>{}, l, N + 1)...);
    }

    static type apply(lua_State *l) {
        auto ret = worker(l, typename _indices_builder<S>::type());
        lua_pop(l, S);
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
        T ret = _get(_id<T>{}, l, -1);
        lua_pop(l, 1);
        return ret;
    }
};

template <typename... T>
typename _pop_n_impl<sizeof...(T), T...>::type _pop_n(lua_State *l) {
    return _pop_n_impl<sizeof...(T), T...>::apply(l);
}

template <std::size_t S, typename... Ts>
struct _pop_n_reset_impl {
    using type =  std::tuple<Ts...>;

    template <std::size_t... N>
    static type worker(lua_State *l,
                       _indices<N...>) {
        return std::make_tuple(_get(_id<Ts>{}, l, N + 1)...);
    }

    static type apply(lua_State *l) {
        auto ret = worker(l, typename _indices_builder<S>::type());
        lua_settop(l, 0);
        return ret;
    }
};

// Popping nothing returns void
template <typename... Ts>
struct _pop_n_reset_impl<0, Ts...> {
    using type = void;
    static type apply(lua_State *l) {
        lua_settop(l, 0);
    }
};

// Popping one element returns an unboxed value
template <typename T>
struct _pop_n_reset_impl<1, T> {
    using type = T;
    static type apply(lua_State *l) {
        T ret = _get(_id<T>{}, l, -1);
        lua_settop(l, 0);
        return ret;
    }
};

template <typename... T>
typename _pop_n_reset_impl<sizeof...(T), T...>::type
_pop_n_reset(lua_State *l) {
    return _pop_n_reset_impl<sizeof...(T), T...>::apply(l);
}

template <typename T>
T _pop(_id<T> t, lua_State *l) {
    T ret =  _get(t, l, -1);
    lua_pop(l, 1);
    return ret;
}

}
}
