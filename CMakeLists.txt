cmake_minimum_required(VERSION 2.8)
project(Selene)

if (${CMAKE_MAJOR_VERSION} LESS 3)
    # hack to support cmake < 3.0.2 because FindLua does not exist
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Selene_SOURCE_DIR}/cmake)
endif()

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
endif(APPLE)

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
