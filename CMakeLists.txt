cmake_minimum_required(VERSION 3.10)
project(ptracewrap)

set(CMAKE_CXX_STANDARD 11)

add_library(ptracewrap INTERFACE)
target_sources(ptracewrap INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include/ptracewrap.hpp")
target_include_directories(ptracewrap INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include/")
