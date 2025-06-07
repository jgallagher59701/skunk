add_executable(opendap)

add_library(handler
        src/handler.cpp
)

target_link_libraries(handler PRIVATE
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

add_executable(test_handler test/test_handler.cpp)
target_link_libraries(test_handler handler httplib GTest::GTest GTest::Main pthread)

add_test(NAME HandlerTest COMMAND test_handler)
