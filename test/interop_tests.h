#pragma once

#include <selene.h>
#include <string>

int my_add(int a, int b) {
    return a + b;
}

void no_return() {
}

bool test_function_no_args() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Call("foo");
    return true;
}

bool test_add() {
    sel::State state;
    state.Load("../test/test.lua");
    return (state.Call<int>("add", 5, 2) == 7);
}

bool test_multi_return() {
    sel::State state;
    state.Load("../test/test.lua");
    int sum, difference;
    std::tie(sum, difference) = state.Call<int, int>("sum_and_difference", 3, 1);
    return (sum == 4 && difference == 2);
}

bool test_heterogeneous_return() {
    sel::State state;
    state.Load("../test/test.lua");
    int x;
    bool y;
    std::string z;
    std::tie(x, y, z) = state.Call<int, bool, std::string>("bar");
    return (x == 4 && y == true && z == "hi");
}

bool test_call_c_function() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("cadd", std::function<int(int, int)>(my_add));
    int answer = state.Call<int>("cadd", 3, 6);
    return (answer == 9);
}

bool test_call_c_fun_from_lua() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("cadd", std::function<int(int, int)>(my_add));
    int answer = state.Call<int>("execute");
    return (answer == 11);
}

bool test_no_return() {
    sel::State state;
    state.Register("no_return", &no_return);
    state.Call("no_return");
    return true;
}

bool test_call_lambda() {
    sel::State state;
    state.Load("../test/test.lua");
    std::function<int(int, int)> mult = [](int x, int y){ return x * y; };
    state.Register("cmultiply", mult);
    int answer = state.Call<int>("cmultiply", 5, 6);
    return (answer == 30);
}

bool test_call_normal_c_fun() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("cadd", &my_add);
    const int answer = state.Call<int>("cadd", 4, 20);
    return (answer == 24);
}

bool test_call_normal_c_fun_many_times() {
    // Ensures there isn't any strange overflow problem or lingering
    // state
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("cadd", &my_add);
    bool result = true;
    for (int i = 0; i < 25; ++i) {
        const int answer = state.Call<int>("cadd", 4, 20);
        result = result && (answer == 24);
    }
    return result;
}

bool test_call_functor() {
    struct the_answer {
        int answer = 42;
        int operator()() {
            return answer;
        }
    };
    the_answer functor;
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("c_the_answer", std::function<int()>(functor));
    int answer = state.Call<int>("c_the_answer");
    return (answer == 42);

}

std::tuple<int, int> my_sum_and_difference(int x, int y) {
    return std::make_tuple(x+y, x-y);
}

bool test_multivalue_c_fun_return() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("test_fun", &my_sum_and_difference);
    int sum, difference;
    std::tie(sum, difference) = state.Call<int, int>("test_fun", -2, 2);
    return (sum == 0 && difference == -4);
}

bool test_multivalue_c_fun_from_lua() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("doozy_c", &my_sum_and_difference);
    int answer = state.Call<int>("doozy", 5);
    return (answer == -75);
}

bool test_c_fun_destructor() {
    sel::State state;
    state.Load("../test/test.lua");
    state.Register("doozy_c", &my_sum_and_difference);
    state.Call<int>("doozy", 5);
    state.Unregister("doozy_c");
    return state.CheckNil("doozy_c");
}
