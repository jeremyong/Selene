cmake_minimum_required(VERSION 2.8)
project(Selene)

set(LUA_INCLUDE_DIR "" CACHE FILEPATH "Path to lua headers")

if(${LUA_INCLUDE_DIR})
  if(NOT EXISTS ${LUA_INCLUDE_DIR}/lua.h)
    message(SEND_ERROR "Can't find lua.h in ${LUA_INCLUDE_DIR}")
  endif()
endif()

include_directories(${LUA_INCLUDE_DIR})

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
endif(APPLE)

include_directories(include)
file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/*.h include/selene/*.h)

add_executable(test_runner ${CMAKE_CURRENT_SOURCE_DIR}/test/Test.cpp)
target_link_libraries(test_runner lua)
