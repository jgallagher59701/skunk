
# Tell CMake to search here first for config files or libraries:
#set(CMAKE_PREFIX_PATH
#		/Users/jimg/src/opendap/hyrax_git/build/deps
#		${CMAKE_PREFIX_PATH}
#)

find_path(NC_INCLUDE_DIR netcdf.h
		HINTS /Users/jimg/src/opendap/hyrax_git/build/deps/include
		/Users/jimg/src/opendap/hyrax/build/deps/include)
find_library(NC_LIB netcdf
		HINTS /Users/jimg/src/opendap/hyrax_git/build/deps/lib
		/Users/jimg/src/opendap/hyrax/build/deps/lib)

message(STATUS "NC_LIB: ${NC_LIB}")

add_executable(opendap)

include_directories(src)
include_directories(src/netcdf)

# Code in subdirs of opendap/src (e.g., netcdf) needs this path. 6/22/25
# set(OPENDAP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src/netcdf)

# made these a local library for testing, but I'm not sure that's needed.
# jhrg 6/16/25
add_library(handler
        src/handler.cpp
        src/DataAccess.cpp
)

target_link_libraries(handler PUBLIC
		# If we switch to the external library, build in the project,
		# replace 'libdap' below with 'libdap_interface' for the 'wrapped'
		# version of the library. jhrg 6/16/25
		libdap
		libdapserver    # ← pull in dapserver API
		netcdf_support
		${NC_LIB}
        httplib  # assuming header-only httplib as an INTERFACE target
)

target_sources(opendap PRIVATE
        src/opendap_service.cpp
)

target_link_libraries(opendap PUBLIC
        handler
		# These are inherited from handler's target_link_libraries above
		# because it uses PUBLIC. 6/24/25 jhrg
		# netcdf_support
		# ${NC_LIB}
        # httplib  # assuming header-only httplib as an INTERFACE target
)

# Add GoogleTest
enable_testing()
find_package(GTest REQUIRED)

add_executable(test_handler test/test_handler.cpp)
target_link_libraries(test_handler handler httplib GTest::GTest GTest::Main pthread)

add_test(NAME HandlerTest COMMAND test_handler)
