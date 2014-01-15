#pragma once

#include <functional>
#include <string>
#include <tuple>
#include "util.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace luna {
namespace detail {
template <std::size_t... Is>
struct _indices {};

template <std::size_t N, std::size_t... Is>
struct _indices_builder : _indices_builder<N-1, N-1, Is...> {};

template <std::size_t... Is>
struct _indices_builder<0, Is...> : _indices<Is...> {
    using type = _indices<Is...>;
};

template <typename Ret, typename... Args, std::size_t... N>
Ret _lift(std::function<Ret(Args...)> fun,
          std::tuple<Args...> args,
          _indices<N...>) {
    return fun(std::get<N>(args)...);
}

template <typename... T, std::size_t... N>
std::tuple<T...> _get_args_impl(lua_State *l, _indices<N...>) {
    return std::make_tuple(_check_get<T>(l, N + 1)...);
}

template <typename... T>
std::tuple<T...> _get_args(lua_State *l) {
    const std::size_t num_args = sizeof...(T);
    std::tuple<T...> ret =
        _get_args_impl<T...>(l, typename _indices_builder<num_args>::type());
    lua_pop(l, num_args);
    return ret;
}

int _lua_dispatcher(lua_State *l);
}

struct BaseFun { virtual int Apply(lua_State *l) = 0; };

template <typename Ret, typename... Args>
class Fun : public BaseFun {
private:
    std::function<Ret(Args...)> _fun;

public:
    Fun(lua_State *l,
        const std::string &name,
        Ret (*fun)(Args...)) : Fun(l, name, std::function<Ret(Args...)>{fun}) {}

    Fun(lua_State *l,
        const std::string &name,
        std::function<Ret(Args...)> fun) : _fun(fun) {
        lua_pushlightuserdata(l, (void *)this);
        lua_pushcclosure(l, &detail::_lua_dispatcher, 1);
        lua_setglobal(l, name.c_str());
    }


    int Apply(lua_State *l) {
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        Ret value = _lift(_fun, args,
                          typename detail::_indices_builder<sizeof...(Args)>::type());
        detail::_push(l, std::forward<Ret>(value));
        return 1;
    }
};
}
