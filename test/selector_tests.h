#pragma once

#include <selene.h>

bool test_select_global() {
    sel::State state;
    state.Load("../test/test.lua");
    int answer = state["my_global"];
    return answer == 4;
}

bool test_select_field() {
    sel::State state;
    state.Load("../test/test.lua");
    lua_Number answer = state["my_table"]["key"];
    return answer == lua_Number(6.4);
}

bool test_select_index() {
    sel::State state;
    state.Load("../test/test.lua");
    std::string answer = state["my_table"][3];
    return answer == "hi";
}

bool test_select_nested_field() {
    sel::State state;
    state.Load("../test/test.lua");
    std::string answer = state["my_table"]["nested"]["foo"];
    return answer == "bar";
}

bool test_select_nested_index() {
    sel::State state;
    state.Load("../test/test.lua");
    int answer = state["my_table"]["nested"][2];
    return answer == -3;
}

bool test_select_equality() {
    sel::State state;
    state.Load("../test/test.lua");
    return state["my_table"]["nested"][2] == -3;
}

bool test_select_cast() {
    sel::State state;
    state.Load("../test/test.lua");
    return int(state["global1"]) == state["global2"];
}
