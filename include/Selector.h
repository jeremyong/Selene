#pragma once

#include <functional>
#include "State.h"
#include <string>
#include <tuple>
#include "util.h"

namespace sel {
class State;
class Selector {
private:
    friend class State;
    std::string _name;
    State &_state;
    using Fun = std::function<void()>;
    using PFun = std::function<void(Fun)>;

    // Traverses the structure up to this element
    Fun _traverse;
    // Pushes this element to the stack
    Fun _get;
    // Sets this element from a function that pushes a value to the
    // stack.
    PFun _put;

    // Functor is stored when the () operator is invoked. The argument
    // is used to indicate how many return values are expected
    using Functor = std::function<void(int)>;
    mutable std::unique_ptr<Functor> _functor;

    Selector(State &s, Fun traverse, Fun get, PFun put)
        : _state(s), _traverse(traverse),
          _get(get), _put(put), _functor{nullptr} {}

    Selector(State &s, const char *name);

    void _check_create_table();
public:

    // Allow automatic casting when used in comparisons
    bool operator==(Selector &other) = delete;

    template <typename... Args>
    const Selector& operator()(Args... args) const {
        auto tuple_args = std::make_tuple(std::forward<Args>(args)...);
        constexpr int num_args = sizeof...(Args);
        auto tmp = new Functor([this, tuple_args, num_args](int num_ret) {
                detail::_push(_state._l, tuple_args);
                lua_call(_state._l, num_args, num_ret);
            });
        _functor.reset(std::move(tmp));
        return *this;
    }

    template <typename... Ret, typename... Args>
    typename detail::_pop_n_impl<sizeof...(Ret), Ret...>::type
    Call(Args&&... args) const {
        _traverse();
        _get();
        detail::_push_n(_state._l, std::forward<Args>(args)...);
        lua_call(_state._l, sizeof...(Args), sizeof...(Ret));
        return detail::_pop_n_reset<Ret...>(_state._l);
    }

    template <typename T>
    void operator=(T t) const {
        _traverse();
        auto push = [this, t]() {
            detail::_push(_state._l, t);
        };
        _put(push);
        lua_settop(_state._l, 0);
    }

    template <typename T, typename... Funs>
    void SetObj(T &t, Funs... funs) {
        _traverse();
        auto fun_tuple = std::make_tuple(funs...);
        auto push = [this, &t, &fun_tuple]() {
            _state.Register(t, fun_tuple);
        };
        _put(push);
        lua_settop(_state._l, 0);
    }

    template <typename T, typename... Args, typename... Funs>
    void SetClass(Funs... funs) {
        _traverse();
        auto fun_tuple = std::make_tuple(funs...);
        auto push = [this, &fun_tuple]() {
            typename detail::_indices_builder<sizeof...(Funs)>::type d;
            _state.RegisterClass<T, Args...>(_name, fun_tuple, d);
        };
        _put(push);
        lua_settop(_state._l, 0);
    }

    template <typename Ret, typename... Args>
    void operator=(std::function<Ret(Args...)> fun) {
        _traverse();
        auto push = [this, fun]() {
            _state.Register(fun);
        };
        _put(push);
    }

    template <typename Ret, typename... Args>
    void operator=(Ret (*fun)(Args...)) {
        _traverse();
        auto push = [this, fun]() {
            _state.Register(fun);
        };
        _put(push);
    }

    void operator=(const char *s) const;

    template <typename... Ret>
    std::tuple<Ret...> GetTuple() const {
        _traverse();
        _get();
        (*_functor)(sizeof...(Ret));
        return detail::_pop_n_reset<Ret...>(_state._l);
    }
    operator bool() const;
    operator int() const;
    operator unsigned int() const;
    operator lua_Number() const;
    operator std::string() const;

    // Chaining operators
    Selector operator[](const char *name);
    Selector operator[](const int index);
};

inline bool operator==(const Selector &s, const char *c) {
    return std::string{c} == std::string(s);
}

inline bool operator==(const char *c, const Selector &s) {
    return std::string{c} == std::string(s);
}

template <typename T>
inline bool operator==(const Selector &s, T&& t) {
    return T(s) == t;
}

template <typename T>
inline bool operator==(T &&t, const Selector &s) {
    return T(s) == t;
}

}
