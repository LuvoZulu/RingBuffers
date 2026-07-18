cmake_minimum_required(VERSION 4.2.3)


file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/gabs/version.hpp" VERSION_FILE)

string(REGEX MATCH "GABS_VERSION_MAJOR ([0-9]+)" _ ${VERSION_FILE})
set(VERSION_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "GABS_VERSION_MINOR ([0-9]+)" _ ${VERSION_FILE})
set(VERSION_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "GABS_VERSION_PATCH ([0-9]+)" _ ${VERSION_FILE})
set(VERSION_PATCH ${CMAKE_MATCH_1})

project(RingBuffers VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}  DESCRIPTION "RingBuffers is a collection of circular containers" LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Catch2 REQUIRED)

add_library(gabs INTERFACE)
target_include_directories(gabs INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


add_executable(tests tests/test_array.cpp)
target_link_libraries(tests PRIVATE gabs Catch2::Catch2WithMain)

message(STATUS "RingBuffers version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")