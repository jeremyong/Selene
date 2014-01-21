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

function execute()
   return cadd(5, 6);
end

function doozy(a)
   x, y = doozy_c(a, 2 * a)
   return x * y
end

mytable = {}
function mytable.foo()
   return 4
end
