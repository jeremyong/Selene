#pragma once

#include "function.h"

/*
 * Extends manipulation of primitives on the stack with more exotic
 * types
 */

namespace sel {
namespace detail {

template <typename R, typename...Args>
inline sel::function<R(Args...)> _check_get(_id<sel::function<R(Args...)>>,
                                            lua_State *l, const int index) {
    lua_pushvalue(l, index);
    return sel::function<R(Args...)>{luaL_ref(l, LUA_REGISTRYINDEX), l};
}

template <typename R, typename... Args>
inline sel::function<R(Args...)> _get(_id<sel::function<R(Args...)>> id,
                                      lua_State *l, const int index) {
    return _check_get(id, l, index);
}

template <typename R, typename... Args>
inline void _push(lua_State *l, sel::function<R(Args...)> fun) {
    fun.Push(l);
}

}
}
