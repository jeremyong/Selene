cmake_minimum_required(VERSION 2.8.12)
project(luab)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -g -Wall")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")
endif(APPLE)

include_directories(include)
file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     src/*.cpp)
file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     include/*.h)

add_library(luab SHARED ${headers} ${sources})

if(APPLE)
    find_library(lua lua)
    target_link_libraries(luab ${lua})
elseif(UNIX)
    target_link_libraries(luab lua)
endif(APPLE)

add_executable(test_call ${CMAKE_CURRENT_SOURCE_DIR}/test/test_call.cpp)
target_link_libraries(test_call luab)
