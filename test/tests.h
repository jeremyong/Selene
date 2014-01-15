#include "luna.h"
#include <cassert>
#include <string>

int my_add(int a, int b) {
    return a + b;
}

bool test_function_no_args() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Call("foo");
    return true;
}

bool test_add() {
    luna::State l;
    l.Load("../test/test.lua");
    return (l.Call<int>("add", 5, 2) == 7);
}

bool test_multi_return() {
    luna::State l;
    l.Load("../test/test.lua");
    int sum, difference;
    std::tie(sum, difference) = l.Call<int, int>("sum_and_difference", 3, 1);
    return (sum == 4 && difference == 2);
}

bool test_heterogeneous_return() {
    luna::State l;
    l.Load("../test/test.lua");
    int x;
    bool y;
    std::string z;
    std::tie(x, y, z) = l.Call<int, bool, std::string>("bar");
    return (x == 4 && y == true && z == "hi");
}

bool test_call_c_function() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("cadd", std::function<int(int, int)>(my_add));
    int answer = l.Call<int>("cadd", 3, 6);
    return (answer == 9);
}

bool test_call_c_fun_from_lua() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("cadd", std::function<int(int, int)>(my_add));
    int answer = l.Call<int>("execute");
    return (answer == 11);
}

bool test_call_lambda() {
    luna::State l;
    l.Load("../test/test.lua");
    std::function<int(int, int)> mult = [](int x, int y){ return x * y; };
    l.Register("cmultiply", mult);
    int answer = l.Call<int>("cmultiply", 5, 6);
    return (answer == 30);
}
