#pragma once

#include <iostream>
#include <memory>
#include <string>
#include "Registry.h"
#include "Selector.h"
#include <tuple>
#include "util.h"
#include <vector>

namespace sel {
class State {
private:
    lua_State *_l;
    bool _l_owner;
    std::unique_ptr<Registry> _registry;

public:
    State() : State(false) {}
    State(bool should_open_libs) : _l(nullptr), _l_owner(true) {
        _l = luaL_newstate();
        if (_l == nullptr) throw 0;
        if (should_open_libs) luaL_openlibs(_l);
        _registry.reset(new Registry(_l));
    }
    State(lua_State *l) : _l(l), _l_owner(false) {
        _registry.reset(new Registry(_l));
    }
    State(const State &other) = delete;
    State &operator=(const State &other) = delete;
    State(State &&other)
        : _l(other._l),
          _l_owner(other._l_owner),
          _registry(std::move(other._registry)) {
        other._l = nullptr;
    }
    State &operator=(State &&other) {
        if (&other == this) return *this;
        _l = other._l;
        _l_owner = other._l_owner;
        _registry = std::move(other._registry);
        other._l = nullptr;
        return *this;
    }
    ~State() {
        if (_l != nullptr && _l_owner) {
            ForceGC();
            lua_close(_l);
        }
        _l = nullptr;
    }

    int Size() const {
        return lua_gettop(_l);
    }

    bool Load(const std::string &file) {
        return !luaL_dofile(_l, file.c_str());
    }

    void OpenLib(const std::string& modname, lua_CFunction openf) {
#if LUA_VERSION_NUM >= 502
        luaL_requiref(_l, modname.c_str(), openf, 1);
#else
        lua_pushcfunction(_l, openf);
        lua_pushstring(_l, modname.c_str());
        lua_call(_l, 1, 0);
#endif
    }

    void Push() {} // Base case

    template <typename T, typename... Ts>
    void Push(T &&value, Ts&&... values) {
        detail::_push(_l, std::forward<T>(value));
        Push(std::forward<Ts>(values)...);
    }

    // Lua stacks are 1 indexed from the bottom and -1 indexed from
    // the top
    template <typename T>
    T Read(const int index) const {
        return detail::_get(detail::_id<T>{}, _l, index);
    }

    bool CheckNil(const std::string &global) {
        lua_getglobal(_l, global.c_str());
        const bool result = lua_isnil(_l, -1);
        lua_pop(_l, 1);
        return result;
    }
public:
    Selector operator[](const char *name) {
        return Selector(_l, *_registry, name);
    }

    bool operator()(const char *code) {
        bool result = !luaL_dostring(_l, code);
        if(result) lua_settop(_l, 0);
        return result;
    }
    void ForceGC() {
        lua_gc(_l, LUA_GCCOLLECT, 0);
    }

    void InteractiveDebug() {
        luaL_dostring(_l, "debug.debug()");
    }

    friend std::ostream &operator<<(std::ostream &os, const State &state);
};

inline std::ostream &operator<<(std::ostream &os, const State &state) {
    os << "sel::State - " << state._l;
    return os;
}
}
