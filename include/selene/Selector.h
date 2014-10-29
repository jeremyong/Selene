#pragma once

#include "exotics.h"
#include <functional>
#include "Registry.h"
#include <string>
#include <tuple>
#include <vector>

namespace sel {
class State;
class Selector {
    friend class State;
private:
    lua_State *_state;
    Registry &_registry;
    std::string _name;
    using Fun = std::function<void()>;
    using PFun = std::function<void(Fun)>;

    // Traverses the structure up to this element
    std::vector<Fun> _traversal;

    // Pushes this element to the stack
    Fun _get;
    // Sets this element from a function that pushes a value to the
    // stack.
    PFun _put;

    // Functor is stored when the () operator is invoked. The argument
    // is used to indicate how many return values are expected
    using Functor = std::function<void(int)>;
    mutable Functor _functor;

    Selector(lua_State *s, Registry &r, const std::string &name,
             std::vector<Fun> traversal, Fun get, PFun put)
        : _state(s), _registry(r), _name(name), _traversal{traversal},
          _get(get), _put(put) {}

    Selector(lua_State *s, Registry &r, const char *name)
        : _state(s), _registry(r), _name(name) {
        _get = [this, name]() {
            lua_getglobal(_state, name);
        };
        _put = [this, name](Fun fun) {
            fun();
            lua_setglobal(_state, name);
        };
    }

    void _check_create_table() const {
        _traverse();
        _get();
        if (lua_istable(_state, -1) == 0 ) { // not table
            lua_pop(_state, 1); // flush the stack
            auto put = [this]() {
                lua_newtable(_state);
            };
            _put(put);
        } else {
            lua_pop(_state, 1);
        }
    }

    void _traverse() const {
        for (auto &fun : _traversal) {
            fun();
        }
    }
public:

    Selector(const Selector &other)
        : _state(other._state),
          _registry(other._registry),
          _name{other._name},
          _traversal{other._traversal},
          _get{other._get},
          _put{other._put},
          _functor(other._functor)
        {}

    ~Selector() {
        // If there is a functor is not empty, execute it and collect no args
        if (_functor) {
            _traverse();
            _get();
            _functor(0);
        }
        lua_settop(_state, 0);
    }

    // Allow automatic casting when used in comparisons
    bool operator==(Selector &other) = delete;

    template <typename... Args>
    const Selector operator()(Args... args) const {
        auto tuple_args = std::make_tuple(std::forward<Args>(args)...);
        constexpr int num_args = sizeof...(Args);
        Selector copy{*this};
        copy._functor = [this, tuple_args, num_args](int num_ret) {
            detail::_push(_state, tuple_args);
            lua_call(_state, num_args, num_ret);
        };
        return copy;
    }

    template <typename L>
    void operator=(L lambda) const {
        _traverse();
        auto push = [this, lambda]() {
            _registry.Register(lambda);
        };
        _put(push);
        lua_settop(_state, 0);
    }


    void operator=(bool b) const {
        _traverse();
        auto push = [this, b]() {
            detail::_push(_state, b);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    void operator=(int i) const {
        _traverse();
        auto push = [this, i]() {
            detail::_push(_state, i);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    void operator=(unsigned int i) const {
        _traverse();
        auto push = [this, i]() {
            detail::_push(_state, i);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    void operator=(lua_Number n) const {
        _traverse();
        auto push = [this, n]() {
            detail::_push(_state, n);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    void operator=(const std::string &s) const {
        _traverse();
        auto push = [this, s]() {
            detail::_push(_state, s);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    template <typename Ret, typename... Args>
    void operator=(std::function<Ret(Args...)> fun) {
        _traverse();
        auto push = [this, fun]() {
            _registry.Register(fun);
        };
        _put(push);
    }

    template <typename Ret, typename... Args>
    void operator=(Ret (*fun)(Args...)) {
        _traverse();
        auto push = [this, fun]() {
            _registry.Register(fun);
        };
        _put(push);
    }

    void operator=(const char *s) const {
        _traverse();
        auto push = [this, s]() {
            detail::_push(_state, std::string{s});
        };
        _put(push);
        lua_settop(_state, 0);
    }

    template <typename T, typename... Funs>
    void SetObj(T &t, Funs... funs) {
        _traverse();
        auto fun_tuple = std::make_tuple(funs...);
        auto push = [this, &t, &fun_tuple]() {
            _registry.Register(t, fun_tuple);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    template <typename T, typename... Args, typename... Funs>
    void SetClass(Funs... funs) {
        _traverse();
        auto fun_tuple = std::make_tuple(funs...);
        auto push = [this, &fun_tuple]() {
            typename detail::_indices_builder<sizeof...(Funs)>::type d;
            _registry.RegisterClass<T, Args...>(_name, fun_tuple, d);
        };
        _put(push);
        lua_settop(_state, 0);
    }

    template <typename... Ret>
    std::tuple<Ret...> GetTuple() const {
        _traverse();
        _get();
        _functor(sizeof...(Ret));
        return detail::_pop_n_reset<Ret...>(_state);
    }

    template <typename T>
    operator T&() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<T*>{}, _state);
        lua_settop(_state, 0);
        return *ret;
    }

    template <typename T>
    operator T*() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<T*>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    operator bool() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<bool>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    operator int() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<int>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    operator unsigned int() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<unsigned int>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    operator lua_Number() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<lua_Number>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    operator std::string() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret =  detail::_pop(detail::_id<std::string>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }

    template <typename R, typename... Args>
    operator sel::function<R(Args...)>() {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret = detail::_pop(detail::_id<sel::function<R(Args...)>>{},
                                _state);
        lua_settop(_state, 0);
        return ret;
    }

    // Chaining operators. If the selector is an rvalue, modify in
    // place. Otherwise, create a new Selector and return it.
    Selector&& operator[](const char *name) && {
        _name += std::string(".") + name;
        _check_create_table();
        _traversal.push_back(_get);
        _get = [this, name]() {
            lua_getfield(_state, -1, name);
        };
        _put = [this, name](Fun fun) {
            fun();
            lua_setfield(_state, -2, name);
            lua_pop(_state, 1);
        };
        return std::move(*this);
    }
    Selector&& operator[](const int index) && {
        _name += std::string(".") + std::to_string(index);
        _check_create_table();
        _traversal.push_back(_get);
        _get = [this, index]() {
            lua_pushinteger(_state, index);
            lua_gettable(_state, -2);
        };
        _put = [this, index](Fun fun) {
            lua_pushinteger(_state, index);
            fun();
            lua_settable(_state, -3);
            lua_pop(_state, 1);
        };
        return std::move(*this);
    }
    Selector operator[](const char *name) const & {
        auto n = _name + "." + name;
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_get);
        Fun get = [this, name]() {
            lua_getfield(_state, -1, name);
        };
        PFun put = [this, name](Fun fun) {
            fun();
            lua_setfield(_state, -2, name);
            lua_pop(_state, 1);
        };
        return Selector{_state, _registry, n, traversal, get, put};
    }
    Selector operator[](const int index) const & {
        auto name = _name + "." + std::to_string(index);
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_get);
        Fun get = [this, index]() {
            lua_pushinteger(_state, index);
            lua_gettable(_state, -2);
        };
        PFun put = [this, index](Fun fun) {
            lua_pushinteger(_state, index);
            fun();
            lua_settable(_state, -3);
            lua_pop(_state, 1);
        };
        return Selector{_state, _registry, name, traversal, get, put};
    }

    friend bool operator==(const Selector &, const char *);

    friend bool operator==(const char *, const Selector &);

private:
    std::string ToString() const {
        _traverse();
        _get();
        if (_functor) {
            _functor(1);
            _functor = nullptr;
        }
        auto ret =  detail::_pop(detail::_id<std::string>{}, _state);
        lua_settop(_state, 0);
        return ret;
    }
};

inline bool operator==(const Selector &s, const char *c) {
    return std::string{c} == s.ToString();
}

inline bool operator==(const char *c, const Selector &s) {
    return std::string{c} == s.ToString();
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
