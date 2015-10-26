#pragma once

namespace sel {

template<typename Class, typename... Args>
class CallOnceMemFun {
    void (Class::*_f)(Args...) const = nullptr;
public:
    CallOnceMemFun() = default;

    CallOnceMemFun(void (Class::*f)(Args...) const) : _f(f) {}

    void operator()(Class const & obj, Args... args) {
        if (_f) {
            auto consume_f = _f;
            _f = nullptr;
            (obj.*consume_f)(std::forward<Args>(args)...);
        }
    }

    operator bool() const {
        return static_cast<bool>(_f);
    }
};

class ResetStackOnScopeExit {
    lua_State * _stack;
    int _saved_top_index;

public:
    explicit ResetStackOnScopeExit(lua_State * stack)
        : _stack(stack),
          _saved_top_index(lua_gettop(_stack))
    {}

    ~ResetStackOnScopeExit() {
        if (_stack) {
            lua_settop(_stack, _saved_top_index);
        }
    }

    ResetStackOnScopeExit(ResetStackOnScopeExit const & ) = delete;
    ResetStackOnScopeExit(ResetStackOnScopeExit       &&) = delete;
    ResetStackOnScopeExit & operator=(ResetStackOnScopeExit const & ) = delete;
    ResetStackOnScopeExit & operator=(ResetStackOnScopeExit       &&) = delete;
};
}
