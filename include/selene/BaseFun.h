#pragma once

#include "exotics.h"
#include <functional>
#include <tuple>

namespace sel {
struct BaseFun {
    virtual ~BaseFun() {}
    virtual int Apply(lua_State *state) = 0;
};

namespace detail {

inline int _lua_dispatcher(lua_State *l) {
    BaseFun *fun = (BaseFun *)lua_touserdata(l, lua_upvalueindex(1));
    return fun->Apply(l);
}

template <typename Ret, typename... Args, std::size_t... N>
inline Ret _lift(std::function<Ret(Args...)> fun,
                 std::tuple<Args...> args,
                 _indices<N...>) {
    return fun(std::get<N>(args)...);
}

template <typename Ret, typename... Args>
inline Ret _lift(std::function<Ret(Args...)> fun,
                 std::tuple<Args...> args) {
    return _lift(fun, args, typename _indices_builder<sizeof...(Args)>::type());
}


template <typename... T, std::size_t... N>
inline std::tuple<T...> _get_args(lua_State *state, _indices<N...>) {
    return std::tuple<T...>{_check_get(_id<T>{}, state, N + 1)...};
}

template <typename... T>
inline std::tuple<T...> _get_args(lua_State *state) {
    constexpr std::size_t num_args = sizeof...(T);
    return _get_args<T...>(state, typename _indices_builder<num_args>::type());
}
}
}
