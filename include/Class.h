#pragma once

#include "ClassFun.h"
#include "Ctor.h"
#include "Dtor.h"
#include <map>
#include <memory>
#include <vector>
#include <stack>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace sel {

struct BaseClass {
    virtual ~BaseClass() {}
};


template <typename T,
          typename A,
          typename... Funs>
class Class : public BaseClass {
private:
    std::string _name;
    std::unique_ptr<A> _ctor;
    std::unique_ptr<Dtor<T>> _dtor;
    std::vector<std::unique_ptr<BaseFun>> _funs;
    std::vector<std::unique_ptr<T>> _instances;

    std::string _get_metatable_name() {
        //return _name + "_lib";
        return _name;
    }

    void _register_ctor(lua_State *state) {
        _ctor.reset(new A(state, _get_metatable_name()));
    }

    void _register_dtor(lua_State *state) {
        _dtor.reset(new Dtor<T>(state, _get_metatable_name()));
    }

    template <typename Ret, typename... Args>
    void _register_fun(lua_State *state,
                       const char *fun_name,
                       Ret(T::*fun)(Args...)) {
        std::function<Ret(T*, Args...)> lambda = [fun](T *t, Args... args) {
            return (t->*fun)(args...);
        };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            new ClassFun<arity, T, Ret, Args...>
            {state, std::string(fun_name), _get_metatable_name(), lambda});
    }

    void _register_funs(lua_State *state) {}

    template <typename F, typename... Fs>
    void _register_funs(lua_State *state,
                        const char *name,
                        F fun,
                        Fs... funs) {
        _register_fun(state, name, fun);
        _register_funs(state, funs...);
    }

public:
    Class(lua_State *state, const std::string &name,
          Funs... funs) : _name(name) {
        luaL_newmetatable(state, _get_metatable_name().c_str());
        _register_ctor(state);
        _register_dtor(state);
        _register_funs(state, funs...);
        lua_pushvalue(state, -1);
        lua_setfield(state, -1, "__index");
    }
};
}
