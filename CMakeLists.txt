cmake_minimum_required(VERSION 3.16)
project(s-works LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cpp-httplib INTERFACE target
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE external/cpp-httplib)

# External Libraries (selective)
add_subdirectory(external/libdap4)
# add_subdirectory(external/bes)

# Your Web Services
add_subdirectory(services/opendap)
add_subdirectory(services/teapot)

