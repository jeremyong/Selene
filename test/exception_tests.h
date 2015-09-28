#pragma once

#include <exception>
#include <lua.h>
#include <selene.h>

bool test_catch_exception_from_callback_within_lua(sel::State &state) {
    state.Load("../test/test_exceptions.lua");
    state["throw_logic_error"] =
        []() {throw std::logic_error("Arbitrary message.");};
    bool ok = true;
    std::string msg;
    sel::tie(ok, msg) = state["call_protected"]("throw_logic_error");
    return !ok && msg == "Arbitrary message.";
}

bool test_catch_unknwon_exception_from_callback_within_lua(sel::State &state) {
    state.Load("../test/test_exceptions.lua");
    state["throw_int"] =
        []() {throw 0;};
    bool ok = true;
    std::string msg;
    sel::tie(ok, msg) = state["call_protected"]("throw_int");
    return !ok && msg == "Caught unknown exception.";
}

bool test_call_exception_handler(sel::State &state) {
    state.Load("../test/test_exceptions.lua");
    int luaStatusCode = LUA_OK;
    std::string message;
    state.HandleExceptionsWith([&luaStatusCode, &message](int s, std::string msg) {
        luaStatusCode = s, message = std::move(msg);
    });
    state["raise"]("Arbitrary message.");
    return luaStatusCode == LUA_ERRRUN
        && message.find("Arbitrary message.") != std::string::npos;
}

bool test_call_exception_handler_for_exception_from_callback(sel::State &state) {
    int luaStatusCode = LUA_OK;
    std::string message;
    state.HandleExceptionsWith([&luaStatusCode, &message](int s, std::string msg) {
        luaStatusCode = s, message = std::move(msg);
    });
    state["throw_logic_error"] =
        []() {throw std::logic_error("Arbitrary message.");};
    state["throw_logic_error"]();
    return luaStatusCode == LUA_ERRRUN
        && message.find("Arbitrary message.") != std::string::npos;
}
