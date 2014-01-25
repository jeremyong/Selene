#pragma once

#include "Class.h"
#include "Fun.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include "util.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

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
    std::map<std::string, std::unique_ptr<BaseClass>> _objs;
public:
    State() : State(false) {}
    State(bool should_open_libs);
    State(const State &other) = delete;
    State &operator=(const State &other) = delete;
    State(State &&other);
    ~State();

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
    template <typename... T>
    typename detail::_pop_n_impl<sizeof...(T), T...>::type Pop() {
        return detail::_pop_n<T...>(_l);
    }

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
    void Register(const std::string &name, T &t,
                  Funs... funs) {
        Unregister(name);
        auto tmp = std::unique_ptr<BaseClass>(
            new Class<T, Funs...>{_l, &t, name, funs...});
        _objs.insert(std::make_pair(name, std::move(tmp)));
    }

    void Unregister(const std::string &name) {
        auto it2 = _objs.find(name);
        if (it2 != _objs.end()) _objs.erase(it2);
    }

    Selector operator[](const char *name);

    friend std::ostream &operator<<(std::ostream &os, const State &state);
};
}
