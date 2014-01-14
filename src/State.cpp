#include "State.h"

namespace luna {
State::State() : _l(nullptr) {
    _l = luaL_newstate();
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

void State::Push(bool &&b) {
    lua_pushboolean(_l, b);
}

void State::Push(int &&i) {
    lua_pushinteger(_l, i);
}

void State::Push(unsigned int &&u) {
    lua_pushunsigned(_l, u);
}

void State::Push(float &&f) {
    lua_pushnumber(_l, f);
}

void State::Push(double &&d) {
    lua_pushnumber(_l, d);
}

void State::Push(std::string &&s) {
    lua_pushlstring(_l, s.c_str(), s.size());
}

template <>
bool State::Read<bool>(const int index) const {
    return lua_toboolean(_l, index);
}

template <>
int State::Read<int>(const int index) const {
    return lua_tointeger(_l, index);
}

template <>
unsigned int State::Read<unsigned int>(const int index) const {
    return lua_tounsigned(_l, index);
}

template <>
float State::Read<float>(const int index) const {
    return lua_tonumber(_l, index);
}

template <>
double State::Read<double>(const int index) const {
    return lua_tonumber(_l, index);
}

template <>
std::string State::Read<std::string>(const int index) const {
    return lua_tostring(_l, index);
}

std::ostream &operator<<(std::ostream &os, const State &state) {
    lua_State *l = state._l;
    int top = lua_gettop(l);
    for (int i = 1; i <= top; ++i) {
        int t = lua_type(l, i);
        switch(t) {
        case LUA_TSTRING:
            os << lua_tostring(l, i);
            break;
        case LUA_TBOOLEAN:
            os << (lua_toboolean(l, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            os << lua_tonumber(l, i);
            break;
        default:
            os << lua_typename(l, t);
            break;
        }
        os << " ";
    }
    return os;
}
}
