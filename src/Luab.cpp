#include "Luab.h"

Luab::Luab() : _l(nullptr) {
    _l = luaL_newstate();
}

Luab::Luab(Luab &&other) : _l(other._l) {
    other._l = nullptr;
}

Luab::~Luab() {
    if (_l == nullptr) return;

    lua_close(_l);
}

bool Luab::Load(const std::string &file) {
    return !luaL_dofile(_l, file.c_str());
}

void Luab::Push(const bool b) {
    lua_pushboolean(_l, b);
}

void Luab::Push(const int i) {
    lua_pushinteger(_l, i);
}

void Luab::Push(const unsigned int u) {
    lua_pushunsigned(_l, u);
}

void Luab::Push(const float f) {
    lua_pushnumber(_l, f);
}

void Luab::Push(const double d) {
    lua_pushnumber(_l, d);
}

void Luab::Push(const std::string &s) {
    lua_pushlstring(_l, s.c_str(), s.size());
}

template <>
bool Luab::Read<bool>(const int index) {
    return lua_toboolean(_l, index);
}

template <>
int Luab::Read<int>(const int index) {
    return lua_tointeger(_l, index);
}

template <>
unsigned int Luab::Read<unsigned int>(const int index) {
    return lua_tounsigned(_l, index);
}

template <>
float Luab::Read<float>(const int index) {
    return lua_tonumber(_l, index);
}

template <>
double Luab::Read<double>(const int index) {
    return lua_tonumber(_l, index);
}

template <>
std::string Luab::Read<std::string>(const int index) {
    return lua_tostring(_l, index);
}

std::ostream &operator<<(std::ostream &os, const Luab &luab) {
    lua_State *l = luab._l;
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
