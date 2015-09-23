#pragma once

#include <exception>
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
