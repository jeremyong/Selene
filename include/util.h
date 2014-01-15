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
template <typename T> T _get(lua_State *l, const int index);

template <typename T> T _check_get(lua_State *l, const int index);

inline void _push(lua_State *l) {} // Necessary in the case no arguments are passed
void _push(lua_State *l, bool &&value);
void _push(lua_State *l, int &&value);
void _push(lua_State *l, unsigned int &&value);
void _push(lua_State *l, float &&value);
void _push(lua_State *l, double &&value);
void _push(lua_State *l, std::string &&value);

// Worker type-trait struct to _pop_n
// Popping multiple elements returns a tuple
template <size_t, typename... Ts> // First template argument denotes
                                  // the sizeof(Ts...)
struct _pop_n_impl {
    typedef std::tuple<Ts...> type;

    template <typename T>
    static std::tuple<T> worker(lua_State *l, const int index) {
        return std::make_tuple(_get<T>(l, index));
    }

    template <typename T1, typename T2, typename... Rest>
    static std::tuple<T1, T2, Rest...> worker(lua_State *l,
                                              const int index) {
        std::tuple<T1> head = std::make_tuple(_get<T1>(l, index));
        return std::tuple_cat(head, worker<T2, Rest...>(l, index + 1));
    }

    static type apply(lua_State *l) {
        auto ret = worker<Ts...>(l, 1);
        lua_pop(l, sizeof...(Ts));
        return ret;
    }
};

// Popping nothing returns void
template <typename... Ts>
struct _pop_n_impl<0, Ts...> {
    typedef void type;
    static type apply(lua_State *) {}
};

// Popping one element returns an unboxed value
template <typename T>
struct _pop_n_impl<1, T> {
    typedef T type;
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
