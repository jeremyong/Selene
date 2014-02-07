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


bool test_register_class(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("print", &Bar::Print, "get_x", &Bar::GetX);
    state.Load("../test/test_class.lua");
    int result1 = state["barx"];
    std::string result2 = state["barp"];
    return result1 == 8 && result2 == "8+2";
}

bool test_class_field_set(sel::State &state) {
    state["Bar"].SetClass<Bar, int>("set", &Bar::SetX, "get", &Bar::GetX);
    state("bar = Bar.new(4)");
    state("x = bar:get()");
    bool check1 = state["x"] == 4;
    state("bar:set(6)");
    state("x = bar:get()");
    bool check2 = state["x"] == 6;
    return check1 && check2;
}

bool test_class_gc(sel::State &state) {
    gc_counter = 0;
    state["GCTest"].SetClass<GCTest>();
    state.Load("../test/test_gc.lua");
    state["make_ten"].Call();
    bool check1 = gc_counter == 10;
    state["destroy_ten"].Call();
    state.ForceGC();
    bool check2 = gc_counter == 0;
    return check1 && check2;
}
