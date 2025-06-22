
add_library(netcdf_support STATIC)

target_sources(netcdf_support PRIVATE
        nc_util.cc		ncdas.cc        ncdds.cc
        NCArray.cc		NCGrid.cc		NCStr.cc
        NCByte.cc		NCInt16.cc		NCStructure.cc
        NCInt32.cc		NCUInt16.cc     NCUInt32.cc
        NCFloat32.cc    NCFloat64.cc	NCSequence.cc
        NCUrl.cc
)

# Set in the top-level CMakeLists file. jhrg 6/21/25

#find_package(PkgConfig REQUIRED)
## pkg_check_modules(NETCDF REQUIRED libnetcdf)   # looks for netcdf_support.pc
#find_package(NetCDF REQUIRED)

#find_path(NC_INCLUDE_DIR netcdf.h
#        HINTS /Users/jimg/src/opendap/hyrax_git/build/deps/include )
#find_library(NC_LIB netcdf
#        HINTS /Users/jimg/src/opendap/hyrax_git/build/deps/lib )

target_include_directories(netcdf_support PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OPENDAP_SRC}
        ${NC_INCLUDE_DIR}               # NetCDF::NetCDF provides this
)

# Link against the NetCDF library
target_link_libraries(netcdf_support
        PUBLIC
        NC_LIB                       # creates an imported target
)

# Optionally propagate any compile definitions NetCDF needs
target_compile_definitions(netcdf_support
        PUBLIC
        ${NetCDF_DEFINITIONS}
)