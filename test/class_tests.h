#pragma once

#include <selene.h>

struct Bar {
    int x;
    Bar(int num) { x = num; }

    std::string Print(int y) {
        return std::to_string(x) + "+" + std::to_string(y);
    }

    void SetX(int x2) {
        x = x2;
    }

    int GetX() {
        return x;
    }
};

struct Zoo {
    int x;
    Zoo(Bar *bar) {
        x = bar->x;
    }
    int GetX() {
        return x;
    }
    void ChangeBar(Bar &bar) {
        bar.x = x * 2;
    }
};

static int gc_counter;
struct GCTest {
    GCTest() {
        ++gc_counter;
    }
    GCTest(const GCTest &other) {
        ++gc_counter;
    }
    ~GCTest() {
        --gc_counter;
    }
};

std::string ShowBarRef(Bar &bar) {
    return std::to_string(bar.x);
}

std::string ShowBarPtr(Bar *bar) {
    return std::to_string(bar->x);
}


bool test_register_class(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("print", &Bar::Print, "get_x", &Bar::GetX);
    state.Load("../test/test_class.lua");
    int result1 = state["barx"];
    std::string result2 = state["barp"];
    return result1 == 8 && result2 == "8+2";
}

bool test_get_member_variable(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("x", &Bar::x);
    state("bar = Bar.new(-2)");
    state("barx = bar:x()");
    state("tmp = bar.x ~= nil");
    return state["barx"] == -2 && state["tmp"];
}

bool test_set_member_variable(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("x", &Bar::x);
    state("bar = Bar.new(-2)");
    state("bar:set_x(-4)");
    state("barx = bar:x()");
    return state["barx"] == -4;
}

bool test_class_field_set(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("set", &Bar::SetX, "get", &Bar::GetX);
    state("bar = Bar.new(4)");
    state("x = bar:get()");
    const bool check1 = state["x"] == 4;
    state("bar:set(6)");
    state("x = bar:get()");
    const bool check2 = state["x"] == 6;
    return check1 && check2;
}

bool test_class_gc(sel::State &state) {
    gc_counter = 0;
    state["GCTest"].SetClass<GCTest>();
    state.Load("../test/test_gc.lua");
    state["make_ten"]();
    const bool check1 = gc_counter == 10;
    state["destroy_ten"]();
    state.ForceGC();
    const bool check2 = gc_counter == 0;
    return check1 && check2;
}

bool test_pass_pointer(sel::State &state) {
    state["Bar"].SetClass<Bar, int>();
    state["Zoo"].SetClass<Zoo, Bar*>("get", &Zoo::GetX);
    state("bar = Bar.new(4)");
    state("zoo = Zoo.new(bar)");
    state("zoox = zoo:get()");
    return state["zoox"] == 4;
}

bool test_pass_ref(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("get", &Bar::GetX);
    state["Zoo"].SetClass<Zoo, Bar*>("change_bar", &Zoo::ChangeBar);
    state("bar = Bar.new(4)");
    state("zoo = Zoo.new(bar)");
    state("zoo:change_bar(bar)");
    state("barx = bar:get()");
    return state["barx"] == 8;
}

bool test_freestanding_fun_ref(sel::State &state) {
    state["Bar"].SetClass<Bar, int>();
    state("bar = Bar.new(4)");
    state["print_bar"] = &ShowBarRef;
    state("barstring = print_bar(bar)");
    return state["barstring"] == "4";
}

bool test_freestanding_fun_ptr(sel::State &state) {
    state["Bar"].SetClass<Bar, int>();
    state("bar = Bar.new(4)");
    state["print_bar"] = &ShowBarPtr;
    state("barstring = print_bar(bar)");
    return state["barstring"] == "4";
}

struct ConstMemberTest {
    const bool foo = true;

    bool get_bool() const {
        return true;
    }
};

bool test_const_member_function(sel::State &state) {
    state["ConstMemberTest"].SetClass<ConstMemberTest>(
        "get_bool", &ConstMemberTest::get_bool);
    state("tmp = ConstMemberTest.new()");
    return state["tmp"];
}

bool test_const_member_variable(sel::State &state) {
    state["ConstMemberTest"].SetClass<ConstMemberTest>(
        "foo", &ConstMemberTest::foo);
    state("tmp1 = ConstMemberTest.new().foo ~= nil");
    state("tmp2 = ConstMemberTest.new().set_foo == nil");
    return state["tmp1"] && state["tmp2"];
}
