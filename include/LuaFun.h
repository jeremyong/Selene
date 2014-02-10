#pragma once
#include "util.h"

namespace sel {
class LuaFun {
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
};
}
