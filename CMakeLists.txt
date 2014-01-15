cmake_minimum_required(VERSION 2.8.12)
project(Luna)

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

add_library(Luna SHARED ${headers} ${sources})

if(APPLE)
    find_library(lua lua)
    target_link_libraries(Luna ${lua})
elseif(UNIX)
    target_link_libraries(Luna lua)
endif(APPLE)

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test/Test.cpp)
target_link_libraries(test Luna)
