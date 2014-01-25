#pragma once

#include "ClassFun.h"
#include <functional>
#include "LuaName.h"
#include <memory>
#include "State.h"
#include <string>
#include "util.h"
#include <vector>

namespace sel {
struct BaseClass {
    virtual ~BaseClass() {}
};
template <typename T, typename... Funs>
class Class : public BaseClass {
private:
    std::vector<std::unique_ptr<BaseFun>> _funs;

    template <typename Ret, typename... Args>
    void _register_fun(lua_State *state,
                       T *t,
                       const char *fun_name,
                       Ret(T::*fun)(Args...)) {
        std::function<Ret(Args...)> lambda = [t, fun](Args... args) {
            return (t->*fun)(args...);
        };
        constexpr int arity = detail::_arity<Ret>::value;
        _funs.emplace_back(
            new ClassFun<arity, Ret, Args...>
            {state, std::string(fun_name), lambda});
    }

    void _register_funs(lua_State *state, T *t) {}

    template <typename F, typename... Fs>
    void _register_funs(lua_State *state, T *t,
                        const char *name,
                        F fun,
                        Fs... funs) {
        _register_fun(state, t, name, fun);
        _register_funs(state, t, funs...);
    }
public:
    Class(lua_State *&state, T *t, Funs... funs) {
        lua_createtable(state, 0, sizeof...(Funs));
        _register_funs(state, t, funs...);
    }
};
}
