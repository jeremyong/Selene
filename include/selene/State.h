#pragma once

#include "exception.h"
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
    std::unique_ptr<ExceptionHandler> _exception_handler;

    /// Shared code post-calling LoadString or LoadFile
    inline bool PostLoad(int status, const char* file = nullptr) {
        std::string filename(file != nullptr ? file : "");
#if LUA_VERSION_NUM >= 502
        auto const lua_ok = LUA_OK;
#else
        auto const lua_ok = 0;
#endif
        if (status != lua_ok) {
            if (status == LUA_ERRSYNTAX) {
                const char *msg = lua_tostring(_l, -1);
                _exception_handler->Handle(status, msg != nullptr ? msg : filename + " : syntax error");
            } else if (status == LUA_ERRFILE) {
                const char *msg = lua_tostring(_l, -1);
                _exception_handler->Handle(status, msg != nullptr ? msg : filename + " : file error");
            }
            lua_remove(_l , -1);
            return false;
        }

        status = lua_pcall(_l, 0, LUA_MULTRET, 0);
        if(status == lua_ok) {
            return true;
        }

        const char *msg = lua_tostring(_l, -1);
        std::string strmsg(msg != nullptr ? msg : (filename.empty() ? "dostring failed" : filename + " : dofile failed"));
        _exception_handler->Handle(status, strmsg);

        lua_remove(_l, -1);
        return false;
    }

public:
    State() : State(false) {}
    State(bool should_open_libs) : _l(nullptr), _l_owner(true), _exception_handler(new ExceptionHandler) {
        _l = luaL_newstate();
        if (_l == nullptr) throw 0;
        if (should_open_libs) luaL_openlibs(_l);
        _registry.reset(new Registry(_l));
        HandleExceptionsPrintingToStdOut();
    }
    State(lua_State *l) : _l(l), _l_owner(false), _exception_handler(new ExceptionHandler) {
        _registry.reset(new Registry(_l));
        HandleExceptionsPrintingToStdOut();
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

    bool Load(const char* file) {
        int status = luaL_loadfile(_l, file);
        return this->PostLoad(status, file);
    }
    
    inline bool Load(const std::string &file) {
        return this->Load(file.c_str());
    }

    bool LoadStr(const char* script) {
        int status = luaL_loadstring(_l, script);
        return this->PostLoad(status);
    }

    inline bool LoadStr(const std::string &script) {
        return this->LoadStr(script.c_str());
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

    void HandleExceptionsPrintingToStdOut() {
        *_exception_handler = ExceptionHandler([](int, std::string msg, std::exception_ptr){_print(msg);});
    }

    void HandleExceptionsWith(ExceptionHandler::function handler) {
        *_exception_handler = ExceptionHandler(std::move(handler));
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
        return Selector(_l, *_registry, *_exception_handler, name);
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
