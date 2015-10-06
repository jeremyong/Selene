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

}
