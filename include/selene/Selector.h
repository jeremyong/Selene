#pragma once

#include "exception.h"
#include "exotics.h"
#include <functional>
#include "LuaRef.h"
#include "Registry.h"
#include <string>
#include <tuple>
#include "util.h"
#include <vector>

#ifdef HAS_REF_QUALIFIERS
# undef HAS_REF_QUALIFIERS
# undef REF_QUAL_LVALUE
#endif

#if defined(__clang__)
# if __has_feature(cxx_reference_qualified_functions)
#  define HAS_REF_QUALIFIERS
# endif
#elif defined(__GNUC__)
# define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
# if GCC_VERSION >= 40801
#  define HAS_REF_QUALIFIERS
# endif
#elif defined(_MSC_VER)
# if _MSC_VER >= 1900 // since MSVS-14 CTP1
#  define HAS_REF_QUALIFIERS
# endif
#endif

#if defined(HAS_REF_QUALIFIERS)
# define REF_QUAL_LVALUE &
#else
# define REF_QUAL_LVALUE
#endif

namespace sel {
class State;
class Selector {
    friend class State;
private:
    lua_State *_state;
    Registry &_registry;
    ExceptionHandler *_exception_handler;
    std::string _name;

    // Traverses the structure up to this element
    std::vector<LuaRef> _traversal;

    // Key of the value to act upon.
    LuaRef _key;

    // Functor is stored when the () operator is invoked. The argument
    // is used to indicate how many return values are expected
    using Functor = std::function<void(int)>;
    mutable Functor _functor;

    Selector(lua_State *s, Registry &r, ExceptionHandler &eh, const std::string &name,
             std::vector<LuaRef> traversal, LuaRef key)
        : _state(s), _registry(r), _exception_handler(&eh), _name(name), _traversal(traversal),
          _key(key) {}

    Selector(lua_State *s, Registry &r, ExceptionHandler &eh, const std::string& name)
        : _state(s), _registry(r), _exception_handler(&eh), _name(name),
          _key(make_Ref(s, name)) {}

    void _get(LuaRef r) const {
        r.Push(_state);
        lua_gettable(_state, -2);
        lua_remove(_state, lua_absindex(_state, -2));
    }

    // Pushes this element to the stack
    void _get() const {
        _get(_key);
    }

    // Sets this element from a function that pushes a value to the
    // stack.
    template<typename PushFunction>
    void _put(PushFunction fun) const {
        _key.Push(_state);
        fun();
        lua_settable(_state, -3);
        lua_pop(_state, 1);
    }

    void _check_create_table() const {
        auto const old_top = lua_gettop(_state);
        _traverse();
        _get();
        if (lua_istable(_state, -1) == 0 ) { // not table
            lua_pop(_state, 1); // flush the stack
            auto put = [this]() {
                lua_newtable(_state);
            };
            _traverse();
            _put(put);
        } else {
            lua_pop(_state, 1);
        }
        lua_settop(_state, old_top);
    }

    void _traverse() const {
        lua_pushglobaltable(_state);
        for (auto &key : _traversal) {
            _get(key);
        }
    }
public:

    Selector(const Selector &other)
        : _state(other._state),
          _registry(other._registry),
          _exception_handler(other._exception_handler),
          _name(other._name),
          _traversal(other._traversal),
          _key(other._key),
          _functor(other._functor)
        {}

    Selector(Selector&& other)
        : _state(other._state),
          _registry(other._registry),
          _exception_handler(other._exception_handler),
          _name(other._name),
          _traversal(other._traversal),
          _key(other._key),
          _functor(other._functor) {
        other._functor = nullptr;
    }

    ~Selector() noexcept(false) {
        // If there is a functor is not empty, execute it and collect no args
        if (_functor) {
            _traverse();
            _get();
            if (std::uncaught_exception())
            {
                try {
                    _functor(0);
                } catch (...) {
                    // We are already unwinding, ignore further exceptions.
                    // As of C++17 consider std::uncaught_exceptions()
                }
            } else {
                _functor(0);
            }
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
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
        const auto eh = _exception_handler;
        copy._functor = [state, eh, tuple_args, num_args](int num_ret) {
            // install handler, and swap(handler, function) on lua stack
            int handler_index = SetErrorHandler(state);
            int func_index = handler_index - 1;
#if LUA_VERSION_NUM >= 502
            lua_pushvalue(state, func_index);
            lua_copy(state, handler_index, func_index);
            lua_replace(state, handler_index);
#else
            lua_pushvalue(state, func_index);
            lua_push_value(state, handler_index);
            lua_replace(state, func_index);
            lua_replace(state, handler_index);
#endif
            // call lua function with error handler
            detail::_push(state, tuple_args);
            auto const statusCode =
                lua_pcall(state, num_args, num_ret, handler_index - 1);

            // remove error handler
            lua_remove(state, handler_index - 1);

            if (statusCode != LUA_OK) {
                eh->Handle_top_of_stack(statusCode, state);
            }
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

    template<
        typename T,
        typename = typename std::enable_if<
            !detail::is_primitive<typename std::decay<T>::type>::value
        >::type
    >
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
        ret._enable_exception_handler(_exception_handler);
        lua_settop(_state, 0);
        return ret;
    }

    // Chaining operators. If the selector is an rvalue, modify in
    // place. Otherwise, create a new Selector and return it.
#ifdef HAS_REF_QUALIFIERS
    Selector&& operator[](const std::string& name) && {
        _name += std::string(".") + name;
        _check_create_table();
        _traversal.push_back(_key);
        _key = make_Ref(_state, name);
        return std::move(*this);
    }
    Selector&& operator[](const char* name) && {
        return std::move(*this)[std::string{name}];
    }
    Selector&& operator[](const int index) && {
        _name += std::string(".") + std::to_string(index);
        _check_create_table();
        _traversal.push_back(_key);
        _key = make_Ref(_state, index);
        return std::move(*this);
    }
#endif // HAS_REF_QUALIFIERS
    Selector operator[](const std::string& name) const REF_QUAL_LVALUE {
        auto n = _name + "." + name;
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_key);
        return Selector{_state, _registry, *_exception_handler, n, traversal, make_Ref(_state, name)};
    }
    Selector operator[](const char* name) const REF_QUAL_LVALUE {
        return (*this)[std::string{name}];
    }
    Selector operator[](const int index) const REF_QUAL_LVALUE {
        auto name = _name + "." + std::to_string(index);
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_key);
        return Selector{_state, _registry, *_exception_handler, name, traversal, make_Ref(_state, index)};
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
