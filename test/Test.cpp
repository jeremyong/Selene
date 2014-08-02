#include <algorithm>
#include "class_tests.h"
#include "obj_tests.h"
#include "interop_tests.h"
#include "reference_tests.h"
#include "selector_tests.h"
#include <map>

// A very simple testing framework
// To add a test, author a function with the Test function signature
// and include it in the Map of tests below.
using Test = bool(*)(sel::State &);
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
    {"test_call_std_fun", test_call_lambda},
    {"test_call_lambda", test_call_lambda},
    {"test_call_normal_c_fun", test_call_normal_c_fun},
    {"test_call_normal_c_fun_many_times", test_call_normal_c_fun_many_times},
    {"test_call_functor", test_call_functor},
    {"test_multivalue_c_fun_return", test_multivalue_c_fun_return},
    {"test_multivalue_c_fun_from_lua", test_multivalue_c_fun_from_lua},
    {"test_embedded_nulls", test_embedded_nulls},
    {"test_coroutine", test_coroutine},
    {"test_pointer_return", test_pointer_return},
    {"test_reference_return", test_reference_return},

    {"test_register_obj", test_register_obj},
    {"test_register_obj_member_variable", test_register_obj_member_variable},
    {"test_register_obj_to_table", test_register_obj_to_table},
    {"test_mutate_instance", test_mutate_instance},
    {"test_multiple_methods", test_multiple_methods},
    {"test_register_obj_const_member_variable", test_register_obj_const_member_variable},

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
    {"test_create_table_index", test_create_table_index},

    {"test_register_class", test_register_class},
    {"test_get_member_variable", test_get_member_variable},
    {"test_set_member_variable", test_set_member_variable},
    {"test_class_field_set", test_class_field_set},
    {"test_class_gc", test_class_gc},
    {"test_pass_pointer", test_pass_pointer},
    {"test_pass_ref", test_pass_ref},
    {"test_freestanding_fun_ref", test_freestanding_fun_ref},
    {"test_freestanding_fun_ptr", test_freestanding_fun_ptr},
    {"test_const_member_function", test_const_member_function},
    {"test_const_member_variable", test_const_member_variable},

    {"test_function_reference", test_function_reference},
    {"test_function_in_constructor", test_function_in_constructor},
    {"test_pass_function_to_lua", test_pass_function_to_lua},
    {"test_call_returned_lua_function", test_call_returned_lua_function},
    {"test_call_multivalue_lua_function", test_call_multivalue_lua_function}
};

// Executes all tests and returns the number of failures.
int ExecuteAll() {
    const int num_tests = tests.size();
    std::cout << "Executing " << num_tests << " tests..." << std::endl;
    std::vector<std::string> failures;
    int passing = 0;
    for (auto it = tests.begin(); it != tests.end(); ++it) {
        sel::State state{true};
        const bool result = it->second(state);
        if (result) {
            passing += 1;
            std::cout << "." << std::flush;
        } else {
            std::cout << "x" << std::flush;
            failures.push_back(std::string{"Test \""} +
                               it->first + "\" failed.");
        }
        int size = state.Size();
        if (size != 0)
            failures.push_back(std::string{"Test \""} + it->first
                               + "\" leaked " + std::to_string(size) + " values");
    }
    std::cout << std::endl << passing << " out of "
              << num_tests << " tests finished successfully." << std::endl;
    std::cout << std::endl;
    for_each(failures.begin(), failures.end(),
             [](std::string failure) {
                 std::cout << failure << std::endl;
             });
    return num_tests - passing;
}

// Not used in general runs. For debugging purposes
bool ExecuteTest(const char *test) {
    sel::State state{true};
    auto it = tests.find(test);
    const bool result = it->second(state);
    const std::string pretty_result = result ? "pass!" : "fail.";
    std::cout << "Ran test " << test << " with result: "
              << pretty_result << std::endl;
    return result;
}


int main() {
    // Executing all tests will run all test cases and check leftover
    // stack size afterwards. It is expected that the stack size
    // post-test is 0.
    return ExecuteAll();

    // For debugging anything in particular, you can run an individual
    //test like so:
    //ExecuteTest("test_pointer_return");
}
