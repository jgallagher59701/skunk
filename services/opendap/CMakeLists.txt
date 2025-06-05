add_executable(opendap)

target_sources(opendap PRIVATE
        src/OpendapService.cpp
        # additional source files
)

target_link_libraries(opendap PRIVATE
        # libdap4
        # bes_http
        # bes_data
        # bes_utility
        httplib  # assuming header-only httplib as an INTERFACE target
)