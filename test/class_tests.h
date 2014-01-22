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

bool test_register_class() {
    Foo foo_instance(1);
    sel::State state;
    state.Register("foo_instance",
                   foo_instance,
                   "double_add", &Foo::DoubleAdd);
    const int answer = state.CallField<int>("foo_instance", "double_add", 3);
    return (answer == 8);
}

bool test_mutate_instance() {
    Foo foo_instance(1);
    sel::State state;
    state.Register("foo_instance",
                   foo_instance,
                   "set_x", &Foo::SetX);
    state.CallField("foo_instance", "set_x", 4);
    return (foo_instance.x == 4);
}

bool test_multiple_methods() {
    Foo foo_instance(1);
    sel::State state;
    state.Register("foo_instance",
                   foo_instance,
                   "double_add", &Foo::DoubleAdd,
                   "set_x", &Foo::SetX);
    state.CallField("foo_instance", "set_x", 4);
    const int answer = state.CallField<int>("foo_instance", "double_add", 3);
    return (answer == 14);
}

bool test_unregister_instance() {
    Foo foo_instance(1);
    sel::State state;
    state.Register("foo_instance",
                   foo_instance,
                   "double_add", &Foo::DoubleAdd,
                   "set_x", &Foo::SetX);
    bool exists = !state.CheckNil("foo_instance");
    state.Unregister("foo_instance");
    return exists && state.CheckNil("foo_instance");
}
