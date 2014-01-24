#pragma once

#include <selene.h>

bool test_select_global() {
    sel::State state;
    state.Load("../test/test.lua");
    int answer = int(state["my_global"]);
    return answer == 4;
}

bool test_select_field() {
    sel::State state;
    state.Load("../test/test.lua");
    lua_Number answer = lua_Number(state["my_table"]["key"]);
    return answer == lua_Number(6.4);
}

bool test_select_index() {
    sel::State state;
    state.Load("../test/test.lua");
    std::string answer = std::string(state["my_table"][3]);
    return answer == "hi";
}

bool test_select_nested_field() {
    sel::State state;
    state.Load("../test/test.lua");
    std::string answer = std::string(state["my_table"]["nested"]["foo"]);
    return answer == "bar";
}

bool test_select_nested_index() {
    sel::State state;
    state.Load("../test/test.lua");
    int answer = int(state["my_table"]["nested"][2]);
    return answer == -3;
}
