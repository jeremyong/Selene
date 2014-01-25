#include <algorithm>
#include "class_tests.h"
#include "interop_tests.h"
#include "selector_tests.h"
#include <map>

// The most ghetto unit testing framework ever!
static std::map<int, bool(*)()> tests = {
    {0, test_function_no_args},
    {1, test_add},
    {2, test_multi_return},
    {3, test_heterogeneous_return},
    {4, test_call_field},
    {5, test_call_c_function},
    {6, test_call_c_fun_from_lua},
    {7, test_no_return},
    {8, test_call_lambda},
    {9, test_call_normal_c_fun},
    {10, test_call_normal_c_fun_many_times},
    {11, test_call_functor},
    {12, test_multivalue_c_fun_return},
    {13, test_multivalue_c_fun_from_lua},
    {15, test_embedded_nulls},

    {16, test_register_class},
    {17, test_mutate_instance},
    {18, test_multiple_methods},
    {19, test_unregister_instance},

    {20, test_select_global},
    {21, test_select_field},
    {22, test_select_index},
    {23, test_select_nested_field},
    {24, test_select_nested_index},
    {25, test_select_equality},
    {26, test_select_cast},
    {27, test_set_global},
    {28, test_set_field},
    {29, test_set_index},
    {30, test_set_nested_field},
    {31, test_set_nested_index}
};


int main() {
    int num_tests = tests.size();
    int passing = 0;
    for (auto it = tests.begin(); it != tests.end(); ++it) {
        if (it->second())
            passing += 1;
        else
            std::cout << "Test " << it->first << " failed." << std::endl;
    }
    std::cout << passing << " out of "
              << num_tests << " tests finished successfully." << std::endl;
}
