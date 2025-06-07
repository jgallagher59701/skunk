add_executable(teapot)

target_sources(teapot PRIVATE
        src/teapot.cpp
)

target_link_libraries(teapot PRIVATE
        httplib  # assuming header-only httplib as an INTERFACE target
)
