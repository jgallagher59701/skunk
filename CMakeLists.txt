cmake_minimum_required(VERSION 3.16)
project(s-works LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cpp-httplib INTERFACE target
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE external/cpp-httplib)

# External Libraries (selective)
include(ExternalProject)

ExternalProject_Add(libdap4
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libdap4
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libdap4/install
        INSTALL_DIR ${CMAKE_BINARY_DIR}/libdap4/install
        BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/libdap4/install/lib/libdap.a  # Adjust if needed
)

ExternalProject_Get_Property(libdap4 install_dir)

# Declare IMPORTED target for libdap
add_library(libdap STATIC IMPORTED GLOBAL)
set_target_properties(libdap PROPERTIES
        IMPORTED_LOCATION ${install_dir}/lib/libdap.a         # Adjust to match actual file
        INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include
)

# Ensure libdap is built before used
add_dependencies(libdap libdap4)

# Link to your own target
#add_executable(my_app src/main.cpp)
#target_link_libraries(my_app PRIVATE libdap)
