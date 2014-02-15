#pragma once

#include <iostream>
#include <selene.h>

int take_fun_arg(sel::function<int(int, int)> fun, int a, int b) {
    return fun(a, b);
}

struct Mutator {
    Mutator() {}
    Mutator(sel::function<void(int)> fun) {
        fun(-4);
    }
    sel::function<void()> Foobar(bool which,
                                 sel::function<void()> foo,
                                 sel::function<void()> bar) {
        return which ? foo : bar;
    }
};

bool test_function_reference(sel::State &state) {
    state["take_fun_arg"] = &take_fun_arg;
    state.Load("../test/test_ref.lua");
    bool check1 = state["pass_add"](3, 5) == 8;
    bool check2 = state["pass_sub"](4, 2) == 2;
    return check1 && check2;
}

bool test_function_in_constructor(sel::State &state) {
    state["Mutator"].SetClass<Mutator, sel::function<void(int)>>();
    state.Load("../test/test_ref.lua");
    bool check1 = state["a"] == 4;
    state("mutator = Mutator.new(mutate_a)");
    bool check2 = state["a"] == -4;
    return check1 && check2;
}

bool test_pass_function_to_lua(sel::State &state) {
    state["Mutator"].SetClass<Mutator>("foobar", &Mutator::Foobar);
    state.Load("../test/test_ref.lua");
    state("mutator = Mutator.new()");
    state("mutator:foobar(true, foo, bar)()");
    bool check1 = state["test"] == "foo";
    state("mutator:foobar(false, foo, bar)()");
    bool check2 = state["test"] == "bar";
    return check1 && check2;
}

bool test_call_returned_lua_function(sel::State &state) {
    state.Load("../test/test_ref.lua");
    sel::function<int(int, int)> lua_add = state["add"];
    return lua_add(2, 4) == 6;
}

bool test_call_multivalue_lua_function(sel::State &state) {
    state.Load("../test/test_ref.lua");
    sel::function<std::tuple<int, int>()> lua_add = state["return_two"];
    return lua_add() == std::make_tuple(1, 2);
}
