#pragma once

#include <tuple>

namespace sel {
template <typename... T>
class Tuple {
private:
    std::tuple<T&...> _tuple;
public:
    Tuple(T&... args) : _tuple(args...) {}

    void operator=(const sel::Selector &s) {
        _tuple = std::tuple<typename std::remove_reference<T>::type...>(s);
    }
};

template <typename... T>
Tuple<T&...> tie(T&... args) {
    return Tuple<T&...>(args...);
}
}
