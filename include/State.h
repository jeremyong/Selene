#pragma once

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

namespace luna {
class State {
private:
    lua_State *_l;

    template<typename T, typename... Rest>
    T read_s() {
        return Read<T>(-1);
    }

    std::map<std::string, std::unique_ptr<BaseFun>> _funs;
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
        return detail::_get<T>(_l, index);
    }

public:
    template <typename... T>
    typename detail::_pop_n_impl<sizeof...(T), T...>::type Pop() {
        return detail::_pop_n<T...>(_l);
    }

    // Calls a lua function with variadic return parameters and
    // function arguments
    template <typename... Ret, typename... Args>
    typename detail::_pop_n_impl<sizeof...(Ret), Ret...>::type
    Call(const std::string &fun, Args&&... args) {
        lua_getglobal(_l, fun.c_str());
        const int num_args = sizeof...(Args);
        const int num_ret = sizeof...(Ret);
        Push(std::forward<Args>(args)...);
        lua_call(_l, num_args, num_ret);
        return Pop<Ret...>();
    }

    template <typename Ret, typename... Args>
    void Register(const std::string &name, std::function<Ret(Args...)> fun) {
        auto tmp = std::unique_ptr<BaseFun>(new Fun<Ret, Args...>{_l, name, fun});
        _funs.insert(std::make_pair(name, std::move(tmp)));
    }

    template <typename Ret, typename... Args>
    void Register(const std::string &name, Ret (*fun)(Args...)) {
        auto tmp = std::unique_ptr<BaseFun>(new Fun<Ret, Args...>{_l, name, fun});
        _funs.insert(std::make_pair(name, std::move(tmp)));
    }

    friend std::ostream &operator<<(std::ostream &os, const State &state);
};
}
