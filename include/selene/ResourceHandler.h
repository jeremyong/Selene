#pragma once

namespace sel {

template<typename Signature>
class CallOnce;

template<typename... Args>
class CallOnce<void(Args...)> {
    std::function<void(Args...)> _f;
public:
    CallOnce() = default;

    CallOnce(std::function<void(Args...)> f) : _f(std::move(f)) {}

    void operator()(Args... args) {
        if(*this) {
            auto consume_f = std::move(_f);
            consume_f(std::forward<Args>(args)...);
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
    : _stack(stack)
    , _saved_top_index(lua_gettop(_stack))
    {}

    void KeepChanges() {
        _stack = nullptr;
    }

    ~ResetStackOnScopeExit() {
        if(_stack) {
            lua_settop(_stack, _saved_top_index);
        }
    }

    ResetStackOnScopeExit(ResetStackOnScopeExit const & ) = delete;
    ResetStackOnScopeExit(ResetStackOnScopeExit       &&) = delete;
    ResetStackOnScopeExit & operator=(ResetStackOnScopeExit const & ) = delete;
    ResetStackOnScopeExit & operator=(ResetStackOnScopeExit       &&) = delete;
};
}
