add_executable(opendap)

# made these a local library for testing, but I'm not sure that's needed.
# jhrg 6/16/25
add_library(handler
        src/handler.cpp
        src/DataAccess.cpp
        src/DataAccessNetCDF.cpp
)

target_link_libraries(handler PRIVATE
		# If we switch to the external library, build in the project,
		# replace 'libdap' below with 'libdap_interface' for the 'wrapped'
		# version of the library. jhrg 6/16/25
		libdap
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

include_directories(src)

add_executable(test_handler test/test_handler.cpp)
target_link_libraries(test_handler handler httplib GTest::GTest GTest::Main pthread)

add_test(NAME HandlerTest COMMAND test_handler)
