#pragma once

#include "Class.h"
#include "exotics.h"
#include "Fun.h"
#include <iostream>
#include <memory>
#include "Obj.h"
#include <string>
#include <tuple>
#include "util.h"
#include <vector>

template <typename T, typename... Rest>
T singular_t();

template <typename... T>
std::tuple<T...> tuple_t();

namespace sel {
class Selector;
class State {
    friend class Selector;
private:
    lua_State *_l;

    std::vector<std::unique_ptr<BaseFun>> _funs;
    std::vector<std::unique_ptr<BaseObj>> _objs;
    std::vector<std::unique_ptr<BaseClass>> _classes;
public:
    State() : State(false) {}
    State(bool should_open_libs);
    State(const State &other) = delete;
    State &operator=(const State &other) = delete;
    State(State &&other);
    ~State();

    int Size() const {
        return lua_gettop(_l);
    }

    bool Load(const std::string &file);

    void Push() {} // Base case

    template <typename T, typename... Ts>
    void Push(T &&value, Ts&&... values) {
        detail::_push(_l, std::forward<T>(value));
        Push(std::forward<Ts>(values)...);
    }

    // Lua stacks are 1 indexed from the bottom and -1 indexed from
    // the top
    template <typename T>
    T Read(const int index) const {
        return detail::_get(detail::_id<T>{}, _l, index);
    }

    bool CheckNil(const std::string &global) {
        lua_getglobal(_l, global.c_str());
        const bool result = lua_isnil(_l, -1);
        lua_pop(_l, 1);
        return result;
    }
public:
    template <typename Ret, typename... Args>
    void Register(std::function<Ret(Args...)> fun) {
        constexpr int arity = detail::_arity<Ret>::value;
        auto tmp = std::unique_ptr<BaseFun>(
            new Fun<arity, Ret, Args...>{_l, fun});
        _funs.push_back(std::move(tmp));
    }

    template <typename Ret, typename... Args>
    void Register(Ret (*fun)(Args...)) {
        constexpr int arity = detail::_arity<Ret>::value;
        auto tmp = std::unique_ptr<BaseFun>(
            new Fun<arity, Ret, Args...>{_l, fun});
        _funs.push_back(std::move(tmp));
    }

    template <typename T, typename... Funs>
    void Register(T &t, std::tuple<Funs...> funs) {
        Register(t, funs,
                 typename detail::_indices_builder<sizeof...(Funs)>::type{});
    }

    template <typename T, typename... Funs, size_t... N>
    void Register(T &t, std::tuple<Funs...> funs, detail::_indices<N...>) {
        RegisterObj(t, std::get<N>(funs)...);
    }

    template <typename T, typename... Funs>
    void RegisterObj(T &t, Funs... funs) {
        auto tmp = std::unique_ptr<BaseObj>(
            new Obj<T, Funs...>{_l, &t, funs...});
        _objs.push_back(std::move(tmp));
    }

    template <typename T, typename... CtorArgs, typename... Funs, size_t... N>
    void RegisterClass(const std::string &name, std::tuple<Funs...> funs,
                       detail::_indices<N...>) {
        RegisterClassWorker<T, CtorArgs...>(name, std::get<N>(funs)...);
    }

    template <typename T, typename... CtorArgs, typename... Funs>
    void RegisterClassWorker(const std::string &name, Funs... funs) {
        auto tmp = std::unique_ptr<BaseClass>(
            new Class<T, Ctor<T, CtorArgs...>, Funs...>
            {_l, name, funs...});
        _classes.push_back(std::move(tmp));
    }

    Selector operator[](const char *name);

    void operator()(const char *code) {
        luaL_dostring(_l, code);
        lua_settop(_l, 0);
    }

    void ForceGC() {
        lua_gc(_l, LUA_GCCOLLECT, 0);
    }

    friend std::ostream &operator<<(std::ostream &os, const State &state);
};
}
