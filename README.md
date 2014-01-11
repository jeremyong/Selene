# luab

Simple C++11 friendly bindings to Lua

## Usage

Currently, these bindings only support calling lua function from C++.

```lua
function foo()
end

function add(a, b)
   return a + b
end

function sum_and_difference(a, b)
   return (a+b), (a-b);
end

function bar()
   return 4, true, "hi"
end
```

```c++
#include "luab.h"
#include <cassert>

int main() {
    Luab l;
    l.Load("../test/test.lua");

    // Call function with no arguments or returns
    l.Call("foo");

    // Call function with two arguments that returns an int
    int result = l.Call<int>("add", 5, 2);
    assert(result == 7);

    // Multiple return types
    int sum, difference;
    std::tie(sum, difference) = l.Call<int, int>("sum_and_difference", 3, 1);
    assert(sum == 4 && difference == 2);

    // Heterogeneous return types
    int x;
    bool y;
    std::string z;
    std::tie(x, y, z) = l.Call<int, bool, std::string>("bar");
    assert(x == 4 && y == true && z == "hi");

    std::cout << "Call tests finished successfully." << std::endl;
}
```

You can read more about this project in the blogpost that describes it [here](http://www.jeremyong.com/blog/2014/01/10/interfacing-lua-with-templates-in-c-plus-plus-11/).