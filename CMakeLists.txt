cmake_minimum_required(VERSION 2.8)
project(Selene)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting cmake build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

if (${CMAKE_MAJOR_VERSION} LESS 3)
    # hack to support cmake < 3.0.2 because FindLua does not exist
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Selene_SOURCE_DIR}/cmake)
endif()

set(CPP_VERSION "c++11" CACHE STRING "C++ version to use")
set_property(CACHE CPP_VERSION PROPERTY STRINGS "c++11" "c++14" "c++1y")
if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=${CPP_VERSION}" HAVE_CPP_VERSION)
    if (NOT HAVE_CPP_VERSION)
        message(FATAL_ERROR "No advanced standard C++ support (-std=${CPP_VERSION} not defined).")
    endif()

    add_definitions("-std=${CPP_VERSION}")
endif()

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

include_directories(include)
file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/*.h include/selene/*.h)

source_group(headers FILES ${headers})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    COMPONENT includes
    FILES_MATCHING
    REGEX "h$"
    )

include(CTest)
add_subdirectory(test)

set(CPACK_GENERATOR "TGZ")
include(CPack)
