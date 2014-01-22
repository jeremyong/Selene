```
  __|        |
\__ \   -_)  |   -_)    \    -_)
____/ \___| _| \___| _| _| \___|
```

Simple C++11 friendly bindings to Lua.

![Lua logo](http://www.lua.org/images/lua-logo.gif)

## Building

The project requires Cmake > v2.8.11 although an older Cmake is likely
to work and creates a shared library.

```
mkdir build
cd build
cmake ..
make
```

This will create an `libSelene.so` library that your application can
link against. Remember to add the `include` directory as well. In
addition, this will build all tests in the `test` directory.

## Usage

### Establishing Lua Context

```c++
using namespace sel;
State state; // creates a Lua context
State state{true]; // creates a Lua context and loads standard Lua libraries
```

When a `sel::State` object goes out of scope, the Lua context is
automatically destroyed in addition to all objects associated with it
(including C++ objects).

### Calling Lua functions from C++

```lua
// test.lua

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

mytable = {}
function mytable.foo()
    return 4
end
```

```c++
sel::State state;
state.Load("/path/to/test.lua");

// Call function with no arguments or returns
state.Call("foo");

// Call function with two arguments that returns an int
int result = state.Call<int>("add", 5, 2);
assert(result == 7);


// Call function that returns multiple values
int sum, difference;
std::tie(sum, difference) = state.Call<int, int>("sum_and_difference", 3, 1);
assert(sum == 4 && difference == 2);

// Call function in table
result = state.CallField<int>("mytable", "foo");
assert(result == 4);
```

### Calling Free-standing C++ functions from Lua

```c++
int my_multiply(int a, int b) {
    return (a*b);
}

sel::State state;

// Register the function to the Lua global "c_multiply"
state.Register("c_multiply", &my_multiply);

// Now we can call it (we can also call it from within lua)
result = state.Call<int>("my_multiply", 5, 2);
assert(result == 10);
```

You can also register functor objects, lambdas, and any fully
qualified `std::function`. See `test/interop_tests.h` for details.

If we no longer want a function to be accessible, we can unregister it
with `state.Unregister("c_multiply")` for example.

### Registering Object Instances

```c++
struct Foo {
    int x;
    Foo(int x_) : x(x_) {}

    int DoubleAdd(int y) { return 2 * (x + y); }
    void SetX(int x_) { x = x_; }
};

sel::State state;

// Instantiate a foo object with x initially set to 2
Foo foo(2);

// Binds the C++ instance foo to a table also called foo in Lua along
// with two methods bound to fields of that table.
// The user is not required to bind all methods
state.Register("foo", foo,
               "double_add", &Foo::DoubleAdd,
               "set_x", &Foo::SetX);

state.CallField("foo", "set_x", 4);
assert(foo.x == 4);

int result = state.CallField("foo", "double_add", 3);
assert(result == 14);

// If foo goes out of scope, we need to unregister it
state.Unregister("foo");
```

In the above example, the functions `foo.double_add` and `foo.set_x`
will also be accessible from within Lua after registration occurs.

## Writeup

You can read more about this project in the blogpost that describes it
[here](http://www.jeremyong.com/blog/2014/01/10/interfacing-lua-with-templates-in-c-plus-plus-11/).
and in the second part
[here](http://www.jeremyong.com/blog/2014/01/14/interfacing-lua-with-templates-in-c-plus-plus-11-continued).

## Roadmap

The following features are planned, although nothing is guaranteed:

- Object lifetime handling
- Smarter Lua module loading
- Hooks for module reloading
