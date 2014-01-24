#include "State.h"

namespace sel {
State::State(bool should_open_libs) : _l(nullptr) {
    _l = luaL_newstate();
    if (_l == nullptr) throw 0;
    if (should_open_libs) luaL_openlibs(_l);
}

State::State(State &&other) : _l(other._l) {
    other._l = nullptr;
}

State::~State() {
    if (_l != nullptr) lua_close(_l);
    _l = nullptr;
}

bool State::Load(const std::string &file) {
    return !luaL_dofile(_l, file.c_str());
}

/*
Selector State::operator[](const char *name) {
    return Selector(_l, name);
}
*/

std::ostream &operator<<(std::ostream &os, const State &state) {
    os << "sel::State - " << state._l;
    return os;
}
}
