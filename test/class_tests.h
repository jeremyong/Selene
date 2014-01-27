#pragma once

#include <selene.h>

struct Foo {
    int x;
    Foo(int x_) : x(x_) {}
    int DoubleAdd(int y) {
        return 2 * (x + y);
    }
    void SetX(int x_) {
        x = x_;
    }
};

bool test_register_class(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance, "double_add", &Foo::DoubleAdd);
    const int answer = state["foo_instance"]["double_add"](3);
    return (answer == 8);
}

bool test_mutate_instance(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance, "set_x", &Foo::SetX);
    state["foo_instance"]["set_x"].Call(4);
    return (foo_instance.x == 4);
}

bool test_multiple_methods(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance,
                                 "double_add", &Foo::DoubleAdd,
                                 "set_x", &Foo::SetX);
    state["foo_instance"]["set_x"].Call(4);
    const int answer = state["foo_instance"]["double_add"](3);
    return (answer == 14);
}
