#pragma once

#include <exception>
#include <utility>

namespace sel {

class SeleneException : public std::exception {};

class TypeError : public SeleneException {
    std::string _message;
public:
    explicit TypeError(std::string expected)
      : _message(std::move(expected)
                 + " expected, got no object.") {}
    explicit TypeError(std::string expected, std::string const & actual)
      : _message(std::move(expected)
                 + " expected, got " + actual + '.') {}
    char const * what() const noexcept override {
        return _message.c_str();
    }
};

}
