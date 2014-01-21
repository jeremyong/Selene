#include <algorithm>
#include "interop_tests.h"
#include "class_tests.h"
#include <vector>

// The most ghetto unit testing framework ever!
static std::vector<bool(*)()> tests = {
    test_function_no_args,
    test_add,
    test_multi_return,
    test_heterogeneous_return,
    test_call_c_function,
    test_call_c_fun_from_lua,
    test_no_return,
    test_call_lambda,
    test_call_normal_c_fun,
    test_call_normal_c_fun_many_times,
    test_call_functor,
    test_multivalue_c_fun_return,
    test_multivalue_c_fun_from_lua,
    test_c_fun_destructor,
    test_register_class,
    test_mutate_instance
};


int main() {
    int num_tests = tests.size();
    int passing = 0;
    std::for_each(tests.begin(), tests.end(),
                  [&passing](bool (*fun)()) {
                      if (fun()) passing += 1;
                  });
    std::cout << passing << " out of "
              << num_tests << " tests finished successfully." << std::endl;
}
