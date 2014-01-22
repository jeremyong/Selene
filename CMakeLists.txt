cmake_minimum_required(VERSION 2.8.12)
project(Selene)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -g -Wall")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")
elseif(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")
endif(APPLE)

option(BUILD_SHARED_LIBS "Build shared libraries." ON)

include_directories(include)
file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     src/*.cpp)
file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     include/*.h)

add_library(Selene ${headers} ${sources})

if(APPLE)
    find_library(lua lua)
    target_link_libraries(Selene ${lua})
elseif(UNIX)
    target_link_libraries(Selene lua)
elseif(WIN32)
    target_link_libraries(Selene lua)
endif(APPLE)

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test/Test.cpp)
target_link_libraries(test Selene)
