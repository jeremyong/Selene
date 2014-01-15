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

bool test_call_normal_c_fun() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("cadd", &my_add);
    int answer = l.Call<int>("cadd", 4, 20);
    return (answer == 24);
}

bool test_call_functor() {
    struct the_answer {
        int answer = 42;
        int operator()() {
            return answer;
        }
    };
    the_answer functor;
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("c_the_answer", std::function<int()>(functor));
    int answer = l.Call<int>("c_the_answer");
    return (answer == 42);

}

std::tuple<int, int> my_sum_and_difference(int x, int y) {
    return std::make_tuple(x+y, x-y);
}

bool test_multivalue_c_fun_return() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("test_fun", &my_sum_and_difference);
    int sum, difference;
    std::tie(sum, difference) = l.Call<int, int>("test_fun", -2, 2);
    return (sum == 0 && difference == -4);
}

bool test_multivalue_c_fun_from_lua() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("doozy_c", &my_sum_and_difference);
    int answer = l.Call<int>("doozy", 5);
    return (answer == -75);
}

bool test_c_fun_destructor() {
    luna::State l;
    l.Load("../test/test.lua");
    l.Register("doozy_c", &my_sum_and_difference);
    l.Call<int>("doozy", 5);
    l.Unregister("doozy_c");
    return l.CheckNil("doozy_c");
}
