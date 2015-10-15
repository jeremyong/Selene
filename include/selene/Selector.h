#pragma once

#include "exception.h"
#include "exotics.h"
#include <functional>
#include "Registry.h"
#include "ResourceHandler.h"
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
    Registry *_registry;
    ExceptionHandler *_exception_handler;
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
    mutable CallOnce<void(int)> _functor;

    Selector(lua_State *s, Registry &r, ExceptionHandler &eh, const std::string &name,
             std::vector<Fun> traversal, Fun get, PFun put)
        : _state(s), _registry(&r), _exception_handler(&eh), _name(name), _traversal(traversal),
          _get(get), _put(put) {}

    Selector(lua_State *s, Registry &r, ExceptionHandler &eh, const std::string& name)
        : _state(s), _registry(&r), _exception_handler(&eh), _name(name) {
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
        // `name' is passed by value because lambda's lifetime may be longer than lifetime of `name'
        _get = [state, name]() {
            lua_getglobal(state, name.c_str());
        };
        _put = [state, name](Fun fun) {
            fun();
            lua_setglobal(state, name.c_str());
        };
    }

    void _check_create_table() const {
        ResetStackOnScopeExit save(_state);
        _traverse();
        _get();
        if (lua_istable(_state, -1) == 0 ) { // not table
            lua_pop(_state, 1); // flush the stack
            auto put = [this]() {
                lua_newtable(_state);
            };
            _put(put);
        }
    }

    void _traverse() const {
        for (auto &fun : _traversal) {
            fun();
        }
    }

    void _evaluate_store(Fun push) const {
        ResetStackOnScopeExit save(_state);
        _traverse();
        _put(std::move(push));
    }

    void _evaluate_retrieve(int num_results) const {
        _traverse();
        _get();
        _functor(num_results);
    }
public:

    Selector(const Selector &) = default;
    Selector(Selector &&) = default;
    Selector & operator=(const Selector &) = default;
    Selector & operator=(Selector &&) = default;

    ~Selector() noexcept(false) {
        // If there is a functor is not empty, execute it and collect no args
        if (_functor) {
            ResetStackOnScopeExit save(_state);
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
        copy._functor = CallOnce<void(int)>{[state, eh, tuple_args, num_args](int num_ret) {
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
        }};
        return copy;
    }

    template <typename L>
    void operator=(L lambda) const {
        _evaluate_store([this, lambda]() {
            _registry->Register(lambda);
        });
    }

    void operator=(bool b) const {
        _evaluate_store([this, b]() {
            detail::_push(_state, b);
        });
    }

    void operator=(int i) const {
        _evaluate_store([this, i]() {
            detail::_push(_state, i);
        });
    }

    void operator=(unsigned int i) const {
        _evaluate_store([this, i]() {
            detail::_push(_state, i);
        });
    }

    void operator=(lua_Number n) const {
        _evaluate_store([this, n]() {
            detail::_push(_state, n);
        });
    }

    void operator=(const std::string &s) const {
        _evaluate_store([this, s]() {
            detail::_push(_state, s);
        });
    }

    template <typename Ret, typename... Args>
    void operator=(std::function<Ret(Args...)> fun) {
        _evaluate_store([this, fun]() {
            _registry->Register(fun);
        });
    }

    template <typename Ret, typename... Args>
    void operator=(Ret (*fun)(Args...)) {
        _evaluate_store([this, fun]() {
            _registry->Register(fun);
        });
    }

    void operator=(const char *s) const {
        _evaluate_store([this, s]() {
            detail::_push(_state, s);
        });
    }

    template <typename T, typename... Funs>
    void SetObj(T &t, Funs... funs) {
        auto fun_tuple = std::make_tuple(std::forward<Funs>(funs)...);
        _evaluate_store([this, &t, &fun_tuple]() {
            _registry->Register(t, fun_tuple);
        });
    }

    template <typename T, typename... Args, typename... Funs>
    void SetClass(Funs... funs) {
        auto fun_tuple = std::make_tuple(std::forward<Funs>(funs)...);
        _evaluate_store([this, &fun_tuple]() {
            typename detail::_indices_builder<sizeof...(Funs)>::type d;
            _registry->RegisterClass<T, Args...>(_name, fun_tuple, d);
        });
    }

    template <typename... Ret>
    std::tuple<Ret...> GetTuple() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(sizeof...(Ret));
        return detail::_get_n<Ret...>(_state);
    }

    template<
        typename T,
        typename = typename std::enable_if<
            !detail::is_primitive<typename std::decay<T>::type>::value
        >::type
    >
    operator T&() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return *detail::_pop(detail::_id<T*>{}, _state);
    }

    template <typename T>
    operator T*() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<T*>{}, _state);
    }

    operator bool() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<bool>{}, _state);
    }

    operator int() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<int>{}, _state);
    }

    operator unsigned int() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<unsigned int>{}, _state);
    }

    operator lua_Number() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<lua_Number>{}, _state);
    }

    operator std::string() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<std::string>{}, _state);
    }

    template <typename R, typename... Args>
    operator sel::function<R(Args...)>() {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        auto ret = detail::_pop(detail::_id<sel::function<R(Args...)>>{},
                                _state);
        ret._enable_exception_handler(_exception_handler);
        return ret;
    }

    // Chaining operators. If the selector is an rvalue, modify in
    // place. Otherwise, create a new Selector and return it.
#ifdef HAS_REF_QUALIFIERS
    Selector&& operator[](const std::string& name) && {
        _name += std::string(".") + name;
        _check_create_table();
        _traversal.push_back(_get);
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
	// `name' is passed by value because lambda lifetime may be longer than `name'
        _get = [state, name]() {
            lua_getfield(state, -1, name.c_str());
        };
        _put = [state, name](Fun fun) {
            fun();
            lua_setfield(state, -2, name.c_str());
            lua_pop(state, 1);
        };
        return std::move(*this);
    }
    Selector&& operator[](const char* name) && {
        return std::move(*this)[std::string{name}];
    }
    Selector&& operator[](const int index) && {
        _name += std::string(".") + std::to_string(index);
        _check_create_table();
        _traversal.push_back(_get);
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
        _get = [state, index]() {
            lua_pushinteger(state, index);
            lua_gettable(state, -2);
        };
        _put = [state, index](Fun fun) {
            lua_pushinteger(state, index);
            fun();
            lua_settable(state, -3);
            lua_pop(state, 1);
        };
        return std::move(*this);
    }
#endif // HAS_REF_QUALIFIERS
    Selector operator[](const std::string& name) const REF_QUAL_LVALUE {
        auto n = _name + "." + name;
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_get);
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
	// `name' is passed by value because lambda lifetime may be longer than `name'
        Fun get = [state, name]() {
            lua_getfield(state, -1, name.c_str());
        };
        PFun put = [state, name](Fun fun) {
            fun();
            lua_setfield(state, -2, name.c_str());
            lua_pop(state, 1);
        };
        return Selector{_state, *_registry, *_exception_handler, n, traversal, get, put};
    }
    Selector operator[](const char* name) const REF_QUAL_LVALUE {
        return (*this)[std::string{name}];
    }
    Selector operator[](const int index) const REF_QUAL_LVALUE {
        auto name = _name + "." + std::to_string(index);
        _check_create_table();
        auto traversal = _traversal;
        traversal.push_back(_get);
        const auto state = _state; // gcc-5.1 doesn't support implicit member capturing
        Fun get = [state, index]() {
            lua_pushinteger(state, index);
            lua_gettable(state, -2);
        };
        PFun put = [state, index](Fun fun) {
            lua_pushinteger(state, index);
            fun();
            lua_settable(state, -3);
            lua_pop(state, 1);
        };
        return Selector{_state, *_registry, *_exception_handler, name, traversal, get, put};
    }

    friend bool operator==(const Selector &, const char *);

    friend bool operator==(const char *, const Selector &);

private:
    std::string ToString() const {
        ResetStackOnScopeExit save(_state);
        _evaluate_retrieve(1);
        return detail::_pop(detail::_id<std::string>{}, _state);
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
