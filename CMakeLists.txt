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
#if(NOT DEFINED ENV{prefix}) 	# NB: No $ENV{} here, but set 'set' below. jhrg 6/16/25
#	set(LIBDAP_PREFIX "/Users/jimg/src/opendap/hyrax/build")
#	message(FATAL_ERROR "The environment variable 'prefix' is not set. Using default: ${LIBDAP_PREFIX}")
#else()
#	set(LIBDAP_PREFIX $ENV{prefix})	# NB: Here use $ENV{} to access the value. Good grief. jhrg 6/16/25
#endif()

set(LIBDAP_PREFIX "/Users/jimg/src/opendap/hyrax_git/build")
message(STATUS "Using LIBDAP_PREFIX=${LIBDAP_PREFIX}")

include_directories(${LIBDAP_PREFIX}/include)
link_directories(${LIBDAP_PREFIX}/lib)

find_library(LIBDAP_LIB
		NAMES dap dapclient dapserver
		PATHS ${LIBDAP_PREFIX}/lib
		REQUIRED
)

add_library(libdap SHARED IMPORTED GLOBAL)
set_target_properties(libdap PROPERTIES
		IMPORTED_LOCATION ${LIBDAP_LIB}
		INTERFACE_INCLUDE_DIRECTORIES ${LIBDAP_PREFIX}/include
)


# Including libdap this way worked but on my machine failed at run-time
# because I have other copies of these libraries from other builds. I
# left this in commented-out because it was a complex configuration to
# work out.
# jhrg 6/16/25

#include(ExternalProject)
#
#set(LIBDAP4_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libdap4)
#set(LIBDAP4_INSTALL_DIR ${CMAKE_BINARY_DIR}/libdap4/install)
#
#if(NOT EXISTS ${LIBDAP4_SOURCE_DIR})
#	message(FATAL_ERROR "External project libdap4 is missing. Did you forget to clone submodules?")
#endif()
#
#ExternalProject_Add(libdap4
#		SOURCE_DIR ${LIBDAP4_SOURCE_DIR}
#		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBDAP4_INSTALL_DIR}
#		INSTALL_DIR ${LIBDAP4_INSTALL_DIR}
#		BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/libdap4/install/lib/libdap.dylib
#			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapclient.dylib
#			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapserver.dylib
#)
#
#ExternalProject_Get_Property(libdap4 install_dir)
#
#add_library(libdap STATIC IMPORTED GLOBAL)
#set_target_properties(libdap PROPERTIES
#		IMPORTED_LOCATION ${LIBDAP4_INSTALL_DIR}/lib/libdap.dylib
#			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapclient.dylib
#			${CMAKE_BINARY_DIR}/libdap4/install/lib/libdapserver.dylib
#		# This breaks things in service/opendap. Replaced with libdap_interface. jhrg 6/16/25
#		# INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${LIBDAP4_INSTALL_DIR}/include>"
#)
#
#add_library(libdap_interface INTERFACE)
#target_include_directories(libdap_interface INTERFACE
#		"$<BUILD_INTERFACE:${LIBDAP4_INSTALL_DIR}/include>"
#)
#target_link_libraries(libdap_interface INTERFACE libdap)
#
#add_dependencies(libdap libdap4)

# ─── Project Subdirectories ────────────────────────────────────────
add_subdirectory(services/opendap)
add_subdirectory(services/teapot)
