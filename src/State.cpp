#include "State.h"

namespace luna {
State::State(bool should_open_libs) : _l(nullptr) {
    _l = luaL_newstate();
    if (should_open_libs) luaL_openlibs(_l);
}

State::State(State &&other) : _l(other._l) {
    other._l = nullptr;
}

State::~State() {
    if (_l == nullptr) return;

    lua_close(_l);
}

bool State::Load(const std::string &file) {
    return !luaL_dofile(_l, file.c_str());
}

std::ostream &operator<<(std::ostream &os, const State &state) {
    os << "luna::State - " << state._l;
    return os;
}
}
