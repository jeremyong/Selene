#pragma once

#include "BaseFun.h"
#include "MetatableRegistry.h"
#include <string>

namespace sel {
template <int N, typename Ret, typename... Args>
class Fun : public BaseFun {
private:
    using _fun_type = std::function<Ret(Args...)>;
    _fun_type _fun;
    MetatableRegistry &_meta_registry;

public:
    Fun(lua_State *&l,
        MetatableRegistry &meta_registry,
        Ret(*fun)(Args...))
        : Fun(l, meta_registry, _fun_type{fun}) {}

    Fun(lua_State *&l,
        MetatableRegistry &meta_registry,
        _fun_type fun) : _fun(fun), _meta_registry(meta_registry) {
        lua_pushlightuserdata(l, (void *)static_cast<BaseFun *>(this));
        lua_pushcclosure(l, &detail::_lua_dispatcher, 1);
    }

    // Each application of a function receives a new Lua context so
    // this argument is necessary.
    int Apply(lua_State *l) override {
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        Ret value = detail::_lift(_fun, args);
        detail::_push(l, _meta_registry, std::forward<Ret>(value));
        return N;
    }

};

template <typename... Args>
class Fun<0, void, Args...> : public BaseFun {
private:
    using _fun_type = std::function<void(Args...)>;
    _fun_type _fun;

public:
    Fun(lua_State *&l,
        MetatableRegistry &dummy,
        void(*fun)(Args...))
        : Fun(l, dummy, _fun_type{fun}) {}

    Fun(lua_State *&l,
        MetatableRegistry &,
        _fun_type fun) : _fun(fun) {
        lua_pushlightuserdata(l, (void *)static_cast<BaseFun *>(this));
        lua_pushcclosure(l, &detail::_lua_dispatcher, 1);
    }

    // Each application of a function receives a new Lua context so
    // this argument is necessary.
    int Apply(lua_State *l) {
        std::tuple<Args...> args = detail::_get_args<Args...>(l);
        detail::_lift(_fun, args);
        return 0;
    }
};
}
