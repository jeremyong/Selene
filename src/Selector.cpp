#include "Selector.h"

namespace sel {
void Selector::_check_create_table() {
    _traverse();
    _get();
    if (lua_istable(_state._l, -1) == 0 ) { // not table
        lua_pop(_state._l, 1); // flush the stack
        auto put = [this]() {
            lua_newtable(_state._l);
        };
        _put(put);
    }
}

Selector::Selector(State &s, const char *name)
    : _state(s), _functor{nullptr} {
    _traverse = [](){};
    _get = [this, name]() {
        lua_getglobal(_state._l, name);
    };
    _put = [this, name](Fun fun) {
        fun();
        lua_setglobal(_state._l, name);
    };
}

void Selector::operator=(const char *s) const {
    _traverse();
    auto push = [this, s]() {
        detail::_push(_state._l, std::string{s});
    };
    _put(push);
}

Selector::operator bool() const {
    _traverse();
    _get();
    if (_functor != nullptr) {
        (*_functor)(1);
        _functor.release();
    }
    return detail::_get(detail::_id<bool>{}, _state._l, -1);
}

Selector::operator int() const {
    _traverse();
    _get();
    if (_functor != nullptr) {
        (*_functor)(1);
        _functor.release();
    }
    return detail::_get(detail::_id<int>{}, _state._l, -1);
}

Selector::operator unsigned int() const {
    _traverse();
    _get();
    if (_functor != nullptr) {
        (*_functor)(1);
        _functor.release();
    }
    return detail::_get(detail::_id<unsigned int>{}, _state._l, -1);
}

Selector::operator lua_Number() const {
    _traverse();
    _get();
    if (_functor != nullptr) {
        (*_functor)(1);
        _functor.release();
    }
    return detail::_get(detail::_id<lua_Number>{}, _state._l, -1);
}

Selector::operator std::string() const {
    _traverse();
    _get();
    if (_functor != nullptr) {
        (*_functor)(1);
        _functor.release();
    }
    return detail::_get(detail::_id<std::string>{}, _state._l, -1);
}

Selector Selector::operator[](const char *name) {
    _check_create_table();
    Fun traverse = [this]() {
        _traverse();
        _get();
    };
    Fun get = [this, name]() {
        lua_getfield(_state._l, -1, name);
    };
    PFun put = [this, name](Fun fun) {
        fun();
        lua_setfield(_state._l, -2, name);
        lua_pop(_state._l, 1);
    };
    return Selector{_state, traverse, get, put};
}

Selector Selector::operator[](const int index) {
    _check_create_table();
    Fun traverse = [this]() {
        _traverse();
        _get();
    };
    Fun get = [this, index]() {
        lua_pushinteger(_state._l, index);
        lua_gettable(_state._l, -2);
    };
    PFun put = [this, index](Fun fun) {
        lua_pushinteger(_state._l, index);
        fun();
        lua_settable(_state._l, -3);
        lua_pop(_state._l, 1);
    };
    return Selector{_state, traverse, get, put};
}
}
