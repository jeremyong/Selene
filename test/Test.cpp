#include <algorithm>
#include "class_tests.h"
#include "interop_tests.h"
#include "selector_tests.h"
#include <map>

// The most ghetto unit testing framework ever!
using Test = bool(*)();
using TestMap = std::map<const char *, Test>;
static TestMap tests = {
    {"test_function_no_args", test_function_no_args},
    {"test_add", test_add},
    {"test_multi_return", test_multi_return},
    {"test_heterogeneous_return", test_heterogeneous_return},
    {"test_call_field", test_call_field},
    {"test_call_c_function", test_call_c_function},
    {"test_call_c_fun_from_lua", test_call_c_fun_from_lua},
    {"test_no_return", test_no_return},
    {"test_call_lambda", test_call_lambda},
    {"test_call_normal_c_fun", test_call_normal_c_fun},
    {"test_call_normal_c_fun_many_times", test_call_normal_c_fun_many_times},
    {"test_call_functor", test_call_functor},
    {"test_multivalue_c_fun_return", test_multivalue_c_fun_return},
    {"test_multivalue_c_fun_from_lua", test_multivalue_c_fun_from_lua},
    {"test_embedded_nulls", test_embedded_nulls},

    {"test_register_class", test_register_class},
    {"test_mutate_instance", test_mutate_instance},
    {"test_multiple_methods", test_multiple_methods},

    {"test_select_global", test_select_global},
    {"test_select_field", test_select_field},
    {"test_select_index", test_select_index},
    {"test_select_nested_field", test_select_nested_field},
    {"test_select_nested_index", test_select_nested_index},
    {"test_select_equality", test_select_equality},
    {"test_select_cast", test_select_cast},
    {"test_set_global", test_set_global},
    {"test_set_field", test_set_field},
    {"test_set_index", test_set_index},
    {"test_set_nested_field", test_set_nested_field},
    {"test_set_nested_index", test_set_nested_index},
    {"test_create_table_field", test_create_table_field},
    {"test_create_table_index", test_create_table_index}
};

void ExecuteAll() {
    const int num_tests = tests.size();
    int passing = 0;
    for (auto it = tests.begin(); it != tests.end(); ++it) {
        const bool result = it->second();
        if (result)
            passing += 1;
        else
            std::cout << "Test \"" << it->first << "\" failed." << std::endl;
    }
    std::cout << passing << " out of "
              << num_tests << " tests finished successfully." << std::endl;
}

bool ExecuteTest(const char *test) {
    auto it = tests.find(test);
    return it->second();
}


int main() {
    ExecuteAll();
}
