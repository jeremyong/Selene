
add_executable(test_runner Test.cpp)
target_link_libraries(test_runner ${LUA_LIBRARIES})
add_test(test_runner test_runner)
