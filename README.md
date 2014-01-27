```
  __|        |
\__ \   -_)  |   -_)    \    -_)
____/ \___| _| \___| _| _| \___|
```

Simple C++11 friendly bindings to Lua.

![Lua logo](http://www.lua.org/images/lua-logo.gif)

Note that this library is still in the alpha stages and may undergo
significant change.

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
addition, this will build all tests in the `test` directory. If you
wish to build a static library, you can do so by supply an option to
cmake in the above steps like so:

```
cmake -DBUILD_SHARED_LIBS=off ..
```

## Usage

### Establishing Lua Context

```c++
using namespace sel;
State state; // creates a Lua context
State state{true}; // creates a Lua context and loads standard Lua libraries
```

When a `sel::State` object goes out of scope, the Lua context is
automatically destroyed in addition to all objects associated with it
(including C++ objects).

### Accessing elements

```lua
-- test.lua
foo = 4
bar = {}
bar[3] = "hi"
bar["key"] = "there"
```

```c++
sel::State state;
state.Load("/path/to/test.lua");
assert(state["foo"] == 4);
assert(state["bar"][3] == "hi");
assert(state["bar"]["key"] == "there";
```

When the `[]` operator is invoked on a `sel::State` object, a
`sel::Selector` object is returned. The `Selector` is type castable to
all the basic types that Lua can return.

If you access the same element frequently, it is recommended that you
cache the selector for fast access later like so:

```c++
auto bar3 = state["bar"][3]; // foo has type sel::Selector
bar3 = 4;
bar3 = 6;
std::cout << int(bar3) << std::endl;
```

### Calling Lua functions from C++

```lua
-- test.lua

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
state["foo"].Call();

// Call function with two arguments that returns an int
// The type parameter can be one of int, lua_Number, std::string,
// bool, or unsigned int
int result = state["add"](5, 2);
assert(result == 7);


// Call function that returns multiple values
int sum, difference;
sel::tie(sum, difference) = state["sum_and_difference"](3, 1);
assert(sum == 4 && difference == 2);

// Call function in table
result = state["mytable"]["foo"]();
assert(result == 4);
```

Generally, the `operator()` implemenation *does not* actually execute
the function call until a typecast occurs. In otherwords, the actual
type of something like `state["add"](5, 2)` is still a
`sel::Selector`. However, if the `Selector` is then typecast, as in a
statement like `int answer = state["add"](5, 2)`, then the invocation
of the function will occur. This is short hand for
`int answer = state["add"].Call<int>(5, 2)`.

Note that `Call`, unlike
`operator()`, will in fact execute the statement immediately and
return. This is because the return types are given to `Call` as a
template function. Without actually performing a cast, `operator()`
has no way to know how many values to retrieve and what types to
retrieve them as. Note that multi-value returns must have `sel::tie`
on the LHS and not `std::tie`. This will create a `sel::Tuple` as
opposed to an `std::tuple` which has the `operator=` implemented for
the selector type.

### Calling Free-standing C++ functions from Lua

```c++
int my_multiply(int a, int b) {
    return (a*b);
}

sel::State state;

// Register the function to the Lua global "c_multiply"
state["c_multiply"] = &my_multiply;

// Now we can call it (we can also call it from within lua)
int result = state["c_multiply"](5, 2);
assert(result == 10);
```

You can also register functor objects, lambdas, and any fully
qualified `std::function`. See `test/interop_tests.h` for details.

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
state["foo"].SetObj(foo,
                    "double_add", &Foo::DoubleAdd,
                    "set_x", &Foo::SetX);

state["foo"]["set_x"].Call(4);
assert(foo.x == 4);

int result = state["foo"]["double_add"](3);
assert(result == 14);
```

In the above example, the functions `foo.double_add` and `foo.set_x`
will also be accessible from within Lua after registration occurs.

## Writeup

You can read more about this project in the three blogposts that describes it:

- [first part](http://www.jeremyong.com/blog/2014/01/10/interfacing-lua-with-templates-in-c-plus-plus-11/).
- [second part](http://www.jeremyong.com/blog/2014/01/14/interfacing-lua-with-templates-in-c-plus-plus-11-continued)
- [third part](http://www.jeremyong.com/blog/2014/01/21/interfacing-lua-with-templates-in-c-plus-plus-11-conclusion)

There have been syntax changes in library usage but the underlying
concepts of variadic template use and generics is the same.

## Roadmap

The following features are planned, although nothing is guaranteed:

- Object lifetime handling
- Smarter Lua module loading
- Hooks for module reloading
