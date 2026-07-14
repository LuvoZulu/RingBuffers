cmake_minimum_required(VERSION 4.2.3)

project(RingBuffers VERSION 0.0.1 DESCRIPTION "RingBuffers is a collection of circular containers" LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(Catch2 REQUIRED)


add_executable(RingBuffers main.cpp)

target_link_libraries(RingBuffers Catch2::Catch2WithMain)