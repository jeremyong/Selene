# Luna

Simple C++11 friendly bindings to Lua

## Building

The project requires Cmake > v2.8.11 although an older Cmake is likely
to work and creates a shared library.

```
mkdir build
cd build
cmake ..
make
```

This will create an `libluna.so` library that your application can
link against. Remember to add the `include` directory as well. In
addition, this will build all any tests in the `test` directory.

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
#include "luna.h"
#include <cassert>

int my_multiply(int a, int b) {
    return (a*b);
}

int main() {
    luna::State l;
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

    // Call C function from Lua
    l.Register("c_multiply", &my_multiply);
    result = l.Call<int>("my_multiply", 5, 2);
    assert(result == 10);

    std::cout << "Call tests finished successfully." << std::endl;
}
```

You can also register functor objects, lambdas, and any fully
qualified `std::function`. See `test/tests.h` for details.

You can read more about this project in the blogpost that describes it [here](http://www.jeremyong.com/blog/2014/01/10/interfacing-lua-with-templates-in-c-plus-plus-11/).

## Roadmap

The following features are planned, although nothing is guaranteed:

- Object oriented type aware table interface with Lua
- Clean way to expose C++ function to Lua
- Smarter Lua module loading
- Hooks for module reloading
- Handling continuations
- Standardize state management
