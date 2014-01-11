#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include "util.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

template <typename T, typename... Rest>
T singular_t();

template <typename... T>
std::tuple<T...> tuple_t();

class Luab {
private:
    lua_State *_l;

    template<typename T, typename... Rest>
    T read_s() {
        return Read<T>(-1);
    }

public:
    Luab();
    Luab(const Luab &other) = delete;
    Luab &operator=(const Luab &other) = delete;
    Luab(Luab &&other);
    ~Luab();

    bool Load(const std::string &file);

    void Push() {}
    void Push(const bool value);
    void Push(const int value);
    void Push(const unsigned int value);
    void Push(const float value);
    void Push(const double value);
    void Push(const std::string &value);

    template <typename T, typename... Ts>
    void Push(const T value, const Ts... values) {
        Push(value);
        Push(values...);
    }

    template <typename T>
    T Read(const int index);

private:
    template <typename... Ts>
    struct _Pop {
        typedef std::tuple<Ts...> type;

        template <typename T>
        static std::tuple<T> worker(Luab &l, const int index) {
            return std::make_tuple(l.Read<T>(index));
        }

        template <typename T1, typename T2, typename... Rest>
        static std::tuple<T1, T2, Rest...> worker(Luab &l, const int index) {
            std::tuple<T1> head = std::make_tuple(l.Read<T1>(index));
            return std::tuple_cat(head, worker<T2, Rest...>(l, index + 1));
        }

        static type apply(Luab &l) {
            auto ret = worker<Ts...>(l, 1);
            lua_pop(l._l, sizeof...(Ts));
            return ret;
        }
    };

    template <typename T>
    struct _Pop<T> {
        typedef T type;
        static type apply(Luab &l) {
            T ret = l.Read<T>(-1);
            lua_pop(l._l, 1);
            return ret;
        }
    };
public:
    template <typename... T>
    typename _Pop<T...>::type Pop() {
        return _Pop<T...>::apply(*this);
    }

    template <typename... Ret, typename... Args>
    typename _Pop<Ret...>::type Call(const std::string &fun,
                                     const Args&... args) {
        lua_getglobal(_l, fun.c_str());
        const int num_args = sizeof...(Args);
        const int num_ret = sizeof...(Ret);
        Push(args...);
        if (luab::check(_l, lua_pcall(_l, num_args, num_ret, 0))) {
            return Pop<Ret...>();
        } else {
            throw 0;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const Luab &luab);
};
