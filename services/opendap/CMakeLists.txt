
# Tell CMake to search here first for config files or libraries:
#set(CMAKE_PREFIX_PATH
#		/Users/jimg/src/opendap/hyrax_git/build/deps
#		${CMAKE_PREFIX_PATH}
#)

find_path(NC_INCLUDE_DIR netcdf.h
		HINTS /Users/jimg/src/opendap/hyrax_git/build/deps/include)
find_library(NC_LIB netcdf
		HINTS /Users/jimg/src/opendap/hyrax_git/build/deps/lib)

add_executable(opendap)

# made these a local library for testing, but I'm not sure that's needed.
# jhrg 6/16/25
add_library(handler
        src/handler.cpp
        src/DataAccess.cpp
)

target_link_libraries(handler PRIVATE
		# If we switch to the external library, build in the project,
		# replace 'libdap' below with 'libdap_interface' for the 'wrapped'
		# version of the library. jhrg 6/16/25
		libdap
		# netcdf_support
        httplib  # assuming header-only httplib as an INTERFACE target
)

target_sources(opendap PRIVATE
        src/opendap_service.cpp
)

target_link_libraries(opendap PRIVATE
        handler
        httplib  # assuming header-only httplib as an INTERFACE target
)

# Code in subdirs of opendap/src (e.g., netcdf) needs this path. 6/22/25
set(OPENDAP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src/netcdf)

# Once the library builds and is useful
# target_link_libraries(your_executable PRIVATE netcdf)

# Add GoogleTest
enable_testing()
find_package(GTest REQUIRED)

include_directories(src)

add_executable(test_handler test/test_handler.cpp)
target_link_libraries(test_handler handler httplib GTest::GTest GTest::Main pthread)

add_test(NAME HandlerTest COMMAND test_handler)
