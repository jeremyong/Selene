#pragma once

#include <selene.h>
#include <vector>

struct Foo {
    int x;
    const int y;
    Foo(int x_) : x(x_), y(3) {}
    int GetX() { return x; }
    int DoubleAdd(int y) {
        return 2 * (x + y);
    }
    void SetX(int x_) {
        x = x_;
    }
};

bool test_register_obj(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance, "double_add", &Foo::DoubleAdd);
    const int answer = state["foo_instance"]["double_add"](3);
    return answer == 8;
}

bool test_register_obj_member_variable(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance, "x", &Foo::x);
    state["foo_instance"]["set_x"](3);
    const int answer = state["foo_instance"]["x"]();
    return answer == 3;
}

bool test_register_obj_to_table(sel::State &state) {
    Foo foo1(1);
    Foo foo2(2);
    Foo foo3(3);
    auto foos = state["foos"];
    foos[1].SetObj(foo1, "get_x", &Foo::GetX);
    foos[2].SetObj(foo2, "get_x", &Foo::GetX);
    foos[3].SetObj(foo3, "get_x", &Foo::GetX);
    const int answer = int(foos[1]["get_x"]()) +
        int(foos[2]["get_x"]()) +
        int(foos[3]["get_x"]());
    return answer == 6;
}

bool test_mutate_instance(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance, "set_x", &Foo::SetX);
    state["foo_instance"]["set_x"](4);
    return foo_instance.x == 4;
}

bool test_multiple_methods(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance,
                                 "double_add", &Foo::DoubleAdd,
                                 "set_x", &Foo::SetX);
    state["foo_instance"]["set_x"](4);
    const int answer = state["foo_instance"]["double_add"](3);
    return answer == 14;
}

bool test_register_obj_const_member_variable(sel::State &state) {
    Foo foo_instance(1);
    state["foo_instance"].SetObj(foo_instance, "y", &Foo::y);
    const int answer = state["foo_instance"]["y"]();
    state("tmp = foo_instance.set_y == nil");
    return answer == 3 && state["tmp"];
}

bool test_bind_vector_push_back(sel::State &state) {
    std::vector<int> test_vector;
    state["vec"].SetObj(test_vector, "push_back",
                        static_cast<void(std::vector<int>::*)(int&&)>(&std::vector<int>::push_back));
    state["vec"]["push_back"](4);
    return test_vector[0] == 4;
}

bool test_bind_vector_push_back_string(sel::State &state) {
    std::vector<std::string> test_vector;
    state["vec"].SetObj(test_vector, "push_back",
                        static_cast<void(std::vector<std::string>::*)(std::string&&)>(&std::vector<std::string>::push_back));
    state["vec"]["push_back"]("hi");
    return test_vector[0] == "hi";
}
