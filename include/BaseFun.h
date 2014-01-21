#pragma once

#include <functional>
#include <tuple>
#include "util.h"

namespace sel {
struct BaseFun {
    virtual ~BaseFun() {}
    virtual int Apply(lua_State *l) = 0;
};

namespace detail {

int _lua_dispatcher(lua_State *l);

template <typename Ret, typename... Args, std::size_t... N>
Ret _lift(std::function<Ret(Args...)> fun,
          std::tuple<Args...> args,
          _indices<N...>) {
    return fun(std::get<N>(args)...);
}

template <typename Ret, typename... Args>
Ret _lift(std::function<Ret(Args...)> fun,
          std::tuple<Args...> args) {
    return _lift(fun, args, typename _indices_builder<sizeof...(Args)>::type());
}


template <typename... T, std::size_t... N>
std::tuple<T...> _get_args(lua_State *l, _indices<N...>) {
    return std::make_tuple(_check_get<T>(l, N + 1)...);
}

template <typename... T>
std::tuple<T...> _get_args(lua_State *l) {
    constexpr std::size_t num_args = sizeof...(T);
    return _get_args<T...>(l, typename _indices_builder<num_args>::type());
}
}
}
