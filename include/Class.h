#pragma once

#include "ClassFun.h"
#include <functional>
#include "LuaName.h"
#include <memory>
#include "State.h"
#include <string>
#include "util.h"
#include <utility>
#include <vector>

namespace sel {
struct BaseClass {
    virtual ~BaseClass() {}
};
template <typename T, typename... Funs>
class Class : public BaseClass {
private:
    LuaName _name;
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
                        std::pair<const char *, F> fun,
                        std::pair<const char *, Fs>... funs) {
        _register_fun(state, t, fun.first, fun.second);
        _register_funs(state, t, funs...);
    }
public:
    Class(lua_State *&state, T *t, const std::string &name,
          std::pair<const char *, Funs>... funs)
        : _name(state, name) {
        lua_createtable(state, 0, sizeof...(Funs));
        _register_funs(state, t, funs...);
        _name.Register();
    }
};
}
