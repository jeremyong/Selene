#pragma once

#include <functional>
#include "LuaGlobal.h"
#include <string>
#include <tuple>
#include "util.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace sel {
namespace detail {
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

int _lua_dispatcher(lua_State *l);
}

struct BaseFun {
    virtual ~BaseFun() {}
    virtual int Apply(lua_State *l) = 0;
};

template <int N, typename Ret, typename... Args>
class Fun : public BaseFun {
private:
    using _return_type = Ret;
    using _fun_type = std::function<Ret(Args...)>;
    _fun_type _fun;
    LuaGlobal _global;

public:
    Fun(lua_State *&l,
        const std::string &name,
        _return_type(*fun)(Args...))
        : Fun(l, name, _fun_type{fun}) {}

    Fun(lua_State *&l,
        const std::string &name,
        _fun_type fun) : _fun(fun), _global(l, name) {
        lua_pushlightuserdata(l, (void *)static_cast<BaseFun *>(this));
        lua_pushcclosure(l, &detail::_lua_dispatcher, 1);
        _global.Register();
    }

    // Each application of a function receives a new Lua context so
    // this argument is necessary.
    int Apply(lua_State *l) {
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        _return_type value = detail::_lift(_fun, args);
        detail::_push(l, std::forward<_return_type>(value));
        return N;
    }
};

}
