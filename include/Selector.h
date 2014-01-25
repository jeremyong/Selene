#pragma once

#include <functional>
#include "State.h"
#include <string>
#include <tuple>
#include "util.h"

extern "C" {
#include "lua.h"
}

namespace sel {
class Selector {
private:
    State &_state;
    using TFun = std::function<void()>;
    using GFun = TFun;
    using PFun = std::function<void(std::function<void(lua_State *)>)>;
    // Traverses the structure up to this element
    TFun _traverse;
    // Pushes this element to the stack
    GFun _get;
    // Sets this element from a function that pushes a value to the stack
    PFun _put;
    mutable std::unique_ptr<std::function<void(int)>> _functor;

    Selector(State &s, TFun traverse, GFun get, PFun put)
        : _state(s), _traverse(traverse),
          _get(get), _put(put), _functor{nullptr} {}

public:
    Selector(State &s, const char *name) : _state(s), _functor{nullptr} {
        _traverse = [](){};
        _get = [this, name]() {
            lua_getglobal(_state._l, name);
        };
        _put = [this, name](std::function<void(lua_State *)> fun) {
            fun(_state._l);
            lua_setglobal(_state._l, name);
        };
    }

    // Allow automatic casting when used in comparisons
    bool operator==(Selector &other) = delete;

    template <typename... Args>
    const Selector& operator()(Args... args) const {
        auto tuple_args = std::make_tuple(std::forward<Args>(args)...);
        constexpr int num_args = sizeof...(Args);
        auto tmp = [this, tuple_args, num_args](int num_ret) {
            detail::_push(_state._l, tuple_args);
            lua_call(_state._l, num_args, num_ret);
        };
        _functor.reset(new std::function<void(int)>(tmp));
        return *this;
    }

    template <typename... Ret, typename... Args>
    typename detail::_pop_n_impl<sizeof...(Ret), Ret...>::type
    Call(Args&&... args) const {
        _traverse();
        _get();
        detail::_push_n(_state._l, std::forward<Args>(args)...);
        lua_call(_state._l, sizeof...(Args), sizeof...(Ret));
        return detail::_pop_n<Ret...>(_state._l);
    }

    template <typename T>
    void operator=(T t) const {
        _traverse();
        auto push = [t](lua_State *l) {
            detail::_push(l, t);
        };
        _put(push);
    }

    template <typename T, typename... Funs>
    void SetObj(T &t, Funs... funs) {
        _traverse();
        auto fun_tuple = std::make_tuple(funs...);
        auto push = [this, &t, &fun_tuple](lua_State *) {
            _state.Register(t, fun_tuple);
        };
        _put(push);
    }

    template <typename Ret, typename... Args>
    void operator=(std::function<Ret(Args...)> fun) {
        _traverse();
        auto push = [this, fun](lua_State *) {
            _state.Register(fun);
        };
        _put(push);
    }

    template <typename Ret, typename... Args>
    void operator=(Ret (*fun)(Args...)) {
        _traverse();
        auto push = [this, fun](lua_State *) {
            _state.Register(fun);
        };
        _put(push);
    }

    void operator=(const char *s) const {
        _traverse();
        auto push = [s](lua_State *l) {
            detail::_push(l, std::string{s});
        };
        _put(push);
    }

    operator bool() const {
        _traverse();
        _get();
        if (_functor != nullptr) {
            (*_functor)(1);
            _functor.release();
        }
        return detail::_get(detail::_id<bool>{}, _state._l, -1);
    }
    operator int() const {
        _traverse();
        _get();
        if (_functor != nullptr) {
            (*_functor)(1);
            _functor.release();
        }
        return detail::_get(detail::_id<int>{}, _state._l, -1);
    }
    operator unsigned int() const {
        _traverse();
        _get();
        if (_functor != nullptr) {
            (*_functor)(1);
            _functor.release();
        }
        return detail::_get(detail::_id<unsigned int>{}, _state._l, -1);
    }
    operator lua_Number() const {
        _traverse();
        _get();
        if (_functor != nullptr) {
            (*_functor)(1);
            _functor.release();
        }
        return detail::_get(detail::_id<lua_Number>{}, _state._l, -1);
    }
    operator std::string() const {
        _traverse();
        _get();
        if (_functor != nullptr) {
            (*_functor)(1);
            _functor.release();
        }
        return detail::_get(detail::_id<std::string>{}, _state._l, -1);
    }

    Selector operator[](const char *name) const {
        TFun traverse = [this]() {
            _traverse();
            _get();
        };
        GFun get = [this, name]() {
            lua_getfield(_state._l, -1, name);
        };
        PFun put = [this, name](std::function<void(lua_State *)> fun) {
            fun(_state._l);
            lua_setfield(_state._l, -2, name);
        };
        return Selector{_state, traverse, get, put};
    }

    Selector operator[](const int index) const {
        TFun traverse = [this]() {
            _traverse();
            _get();
        };
        GFun get = [this, index]() {
            lua_pushinteger(_state._l, index);
            lua_gettable(_state._l, -2);
        };
        PFun put = [this, index](std::function<void(lua_State *)> fun) {
            lua_pushinteger(_state._l, index);
            fun(_state._l);
            lua_settable(_state._l, -3);
        };
        return Selector{_state, traverse, get, put};
    }
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
