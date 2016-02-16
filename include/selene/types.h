#pragma once

#include "traits.h"
#include <type_traits>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace sel {

namespace detail{

template<typename T>
struct type_t;

template <typename T>
using type = type_t<typename std::decay<T>::type>;

// Marks types Lua offers raw pointer access to.
struct referenceable_tag {};

template <typename T>
using is_referenceable = typename std::is_base_of<referenceable_tag, type<T>>::type;

template <typename T>
using decay_non_referencable =
    typename std::conditional<
        is_referenceable<T>::value,
        T,
        typename std::decay<T>::type
    >::type;

template <typename T> struct _id {};

template<typename T>
T _get(_id<T> id, lua_State *l, const int index) {
    return type<T>{}._get(id, l, index);
}

template<typename T>
T _check_get(_id<T> id, lua_State *l, const int index) {
    return type<T>{}._check_get(id, l, index);
}

// Worker type-trait struct to _get_n
// Getting multiple elements returns a tuple
template <typename... Ts>
struct _get_n_impl {
    using type = std::tuple<Ts...>;

    template <std::size_t... N>
    static type worker(lua_State *l,
                       _indices<N...>) {
        return std::make_tuple(::sel::detail::_get(_id<Ts>{}, l, N + 1)...);
    }

    static type apply(lua_State *l) {
        return worker(l, typename _indices_builder<sizeof...(Ts)>::type());
    }
};

// Getting nothing returns void
template <>
struct _get_n_impl<> {
    using type = void;
    static type apply(lua_State *) {}
};

// Getting one element returns an unboxed value
template <typename T>
struct _get_n_impl<T> {
    using type = T;
    static type apply(lua_State *l) {
        return ::sel::detail::_get(_id<T>{}, l, -1);
    }
};

template <typename... T>
typename _get_n_impl<T...>::type _get_n(lua_State *l) {
    return _get_n_impl<T...>::apply(l);
}

template <typename T>
T _pop(_id<T> id, lua_State *l) {
    T ret =  ::sel::detail::_get(id, l, -1);
    lua_pop(l, 1);
    return ret;
}

template <typename T>
void _push(lua_State *l, T&& t) {
    type<T>{}._push(l, std::forward<T>(t));
}

inline void _push_n(lua_State *) {}

template <typename T, typename... Rest>
inline void _push_n(lua_State *l, T &&value, Rest&&... rest) {
    ::sel::detail::_push(l, std::forward<T>(value));
    ::sel::detail::_push_n(l, std::forward<Rest>(rest)...);
}
}

}
