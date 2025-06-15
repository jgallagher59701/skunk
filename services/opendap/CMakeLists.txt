add_executable(opendap)

add_library(handler
        src/handler.cpp
        src/DataAccess.cpp
        src/DataAccessNetCDF.cpp
)

target_link_libraries(handler PRIVATE
		# This is a trick to get cmake configuration to work. See the top-level
		# CMakeLists file. jhrg 6/16/25
		libdap_interface
        httplib  # assuming header-only httplib as an INTERFACE target
)

target_sources(opendap PRIVATE
        src/opendap_service.cpp
)

target_link_libraries(opendap PRIVATE
        handler
        httplib  # assuming header-only httplib as an INTERFACE target
)

# Add GoogleTest
enable_testing()
find_package(GTest REQUIRED)

# Include httplib (assumed to be in your project root or via package manager)
include_directories(src)
#${CMAKE_BINARY_DIR}/libdap4/install/include)

add_executable(test_handler test/test_handler.cpp)
target_link_libraries(test_handler handler httplib GTest::GTest GTest::Main pthread)

add_test(NAME HandlerTest COMMAND test_handler)
