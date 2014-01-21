#pragma once

#include <selene.h>

struct Foo {
    int x;
    Foo(int x_) : x(x_) {}
    int DoubleAdd(int y) {
        return 2 * (x + y);
    }
    void SetX(int x2) {
        x = x2;
    }
};

bool test_register_class() {
    Foo foo_instance(1);
    sel::State state;
    state.Register("foo_instance",
                   foo_instance,
                   std::make_pair("double_add", &Foo::DoubleAdd));
    int answer = state.CallField<int>("foo_instance", "double_add", 3);
    return (answer == 8);
}

bool test_mutate_instance() {
    Foo foo_instance(1);
    sel::State state;
    state.Register("foo_instance",
                   foo_instance,
                   std::make_pair("set_x", &Foo::SetX));
    state.CallField("foo_instance", "set_x", 4);
    return (foo_instance.x == 4);
}
