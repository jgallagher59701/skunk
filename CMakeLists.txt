cmake_minimum_required(VERSION 3.16)
project(s-works LANGUAGES CXX)

# ─── C++ Configuration ─────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Local Header-Only Libraries ───────────────────────────────────
# cpp-httplib INTERFACE target
add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE external/cpp-httplib)

# ─── External Dependencies ─────────────────────────────────────────
include(ExternalProject)

set(LIBDAP4_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libdap4)
set(LIBDAP4_INSTALL_DIR ${CMAKE_BINARY_DIR}/libdap4/install)

if(NOT EXISTS ${LIBDAP4_SOURCE_DIR})
	message(FATAL_ERROR "External project libdap4 is missing. Did you forget to clone submodules?")
endif()

ExternalProject_Add(libdap4
		SOURCE_DIR ${LIBDAP4_SOURCE_DIR}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBDAP4_INSTALL_DIR}
		INSTALL_DIR ${LIBDAP4_INSTALL_DIR}
		BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/libdap4/install/lib/libdap.dylib
			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapclient.dylib
			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapserver.dylib
)

ExternalProject_Get_Property(libdap4 install_dir)

add_library(libdap STATIC IMPORTED GLOBAL)
set_target_properties(libdap PROPERTIES
		IMPORTED_LOCATION ${LIBDAP4_INSTALL_DIR}/lib/libdap.dylib
			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapclient.dylib
			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapserver.dylib
		# This still breaks things in service/opendap. Replaced with libdap_interface. jhrg 6/16/25
		# INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${LIBDAP4_INSTALL_DIR}/include>"
)

add_library(libdap_interface INTERFACE)
target_include_directories(libdap_interface INTERFACE
		"$<BUILD_INTERFACE:${LIBDAP4_INSTALL_DIR}/include>"
)
target_link_libraries(libdap_interface INTERFACE libdap)

add_dependencies(libdap libdap4)

# ─── Project Subdirectories ────────────────────────────────────────
add_subdirectory(services/opendap)
add_subdirectory(services/teapot)
