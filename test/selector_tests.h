#pragma once

#include <selene.h>

bool test_select_global(sel::State &state) {
    state.Load("../test/test.lua");
    int answer = state["my_global"];
    return answer == 4;
}

bool test_select_field(sel::State &state) {
    state.Load("../test/test.lua");
    lua_Number answer = state["my_table"]["key"];
    return answer == lua_Number(6.4);
}

bool test_select_index(sel::State &state) {
    state.Load("../test/test.lua");
    std::string answer = state["my_table"][3];
    return answer == "hi";
}

bool test_select_nested_field(sel::State &state) {
    state.Load("../test/test.lua");
    std::string answer = state["my_table"]["nested"]["foo"];
    return answer == "bar";
}

bool test_select_nested_index(sel::State &state) {
    state.Load("../test/test.lua");
    int answer = state["my_table"]["nested"][2];
    return answer == -3;
}

bool test_select_equality(sel::State &state) {
    state.Load("../test/test.lua");
    return state["my_table"]["nested"][2] == -3;
}

bool test_select_cast(sel::State &state) {
    state.Load("../test/test.lua");
    return int(state["global1"]) == state["global2"];
}

bool test_set_global(sel::State &state) {
    state.Load("../test/test.lua");
    auto lua_dummy_global = state["dummy_global"];
    lua_dummy_global = 32;
    return state["dummy_global"] == 32;
}

bool test_set_field(sel::State &state) {
    state.Load("../test/test.lua");
    state["my_table"]["dummy_key"] = "testing";
    return state["my_table"]["dummy_key"] == "testing";
}

bool test_set_index(sel::State &state) {
    state.Load("../test/test.lua");
    state["my_table"][10] = 3;
    return state["my_table"][10] == 3;
}

bool test_set_nested_field(sel::State &state) {
    state.Load("../test/test.lua");
    state["my_table"]["nested"]["asdf"] = true;
    return state["my_table"]["nested"]["asdf"];
}

bool test_set_nested_index(sel::State &state) {
    state.Load("../test/test.lua");
    state["my_table"]["nested"][1] = 2;
    return state["my_table"]["nested"][1] == 2;
}

bool test_create_table_field(sel::State &state) {
    state["new_table"]["test"] = 4;
    return state["new_table"]["test"] == 4;
}

bool test_create_table_index(sel::State &state) {
    state["new_table"][3] = 4;
    return state["new_table"][3] == 4;
}

bool test_cache_selector_field_assignment(sel::State &state) {
    sel::Selector s = state["new_table"][3];
    s = 4;
    return state["new_table"][3] == 4;
}

bool test_cache_selector_field_access(sel::State &state) {
    state["new_table"][3] = 4;
    sel::Selector s = state["new_table"][3];
    return s == 4;
}

bool test_cache_selector_function(sel::State &state) {
    state.Load("../test/test.lua");
    sel::Selector s = state["set_global"];
    s();
    return state["global1"] == 8;
}

bool test_function_should_run_once(sel::State &state) {
    state.Load("../test/test.lua");
    auto should_run_once = state["should_run_once"];
    should_run_once();
    return state["should_be_one"] == 1;
}
