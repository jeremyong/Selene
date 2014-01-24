#pragma once

#include <functional>
#include <string>
#include "util.h"

extern "C" {
#include "lua.h"
}

namespace sel {
class Selector {
private:
    lua_State *_l;
    using TFun = std::function<void()>;
    using GFun = TFun;
    using PFun = std::function<void(std::function<void(lua_State *)>)>;
    // Traverses the structure up to this element
    TFun _traverse;
    // Pushes this element to the stack
    GFun _get;
    // Sets this element from a function that pushes a value to the stack
    PFun _put;

    Selector(lua_State *l, TFun traverse, GFun get, PFun put)
        : _l(l), _traverse(traverse), _get(get), _put(put) {}

public:
    Selector(lua_State *l, const char *name) : _l{l} {
        _traverse = [](){};
        _get = [this, name]() {
            lua_getglobal(_l, name);
        };
        _put = [this, name](std::function<void(lua_State *)> fun) {
            fun(_l);
            lua_setglobal(_l, name);
        };
    }

    // Allow automatic casting when used in comparisons
    bool operator==(Selector &other) = delete;

    void operator=(const char *s) const {
        _traverse();
        auto push = [s](lua_State *l) {
            detail::_push(l, std::string{s});
        };
        _put(push);
    }

    template <typename T>
    void operator=(T t) const {
        _traverse();
        auto push = [t](lua_State *l) {
            detail::_push(l, t);
        };
        _put(push);
    }

    operator bool() const {
        _traverse();
        _get();
        return detail::_get<bool>(_l, -1);
    }
    operator int() const {
        _traverse();
        _get();
        return detail::_get<int>(_l, -1);
    }
    operator unsigned int() const {
        _traverse();
        _get();
        return detail::_get<unsigned int>(_l, -1);
    }
    operator lua_Number() const {
        _traverse();
        _get();
        return detail::_get<lua_Number>(_l, -1);
    }
    operator std::string() const {
        _traverse();
        _get();
        return detail::_get<std::string>(_l, -1);
    }

    Selector operator[](const char *name) const {
        TFun traverse = [this]() {
            _traverse();
            _get();
        };
        GFun get = [this, name]() {
            lua_getfield(_l, -1, name);
        };
        PFun put = [this, name](std::function<void(lua_State *)> fun) {
            fun(_l);
            lua_setfield(_l, -2, name);
        };
        return Selector{_l, traverse, get, put};
    }

    Selector operator[](const int index) const {
        TFun traverse = [this]() {
            _traverse();
            _get();
        };
        GFun get = [this, index]() {
            lua_pushinteger(_l, index);
            lua_gettable(_l, -2);
        };
        PFun put = [this, index](std::function<void(lua_State *)> fun) {
            lua_pushinteger(_l, index);
            fun(_l);
            lua_settable(_l, -3);
        };
        return Selector{_l, traverse, get, put};
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
