
add_library(netcdf STATIC)

target_sources(netcdf PRIVATE
        nc_util.cc		ncdas.cc        ncdds.cc
        NCArray.cc		NCGrid.cc		NCStr.cc
        NCByte.cc		NCInt16.cc		NCStructure.cc
        NCInt32.cc		NCUInt16.cc     NCUInt32.cc
        NCFloat32.cc    NCFloat64.cc	NCSequence.cc
        NCUrl.cc
)

find_package(PkgConfig REQUIRED)
# pkg_check_modules(NETCDF REQUIRED libnetcdf)   # looks for netcdf.pc
find_package(NetCDF REQUIRED)

target_include_directories(netcdf PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${NetCDF_INCLUDE_DIRS}               # NetCDF::NetCDF provides this
)

# Link against the NetCDF library
target_link_libraries(netcdf
        PUBLIC
        NetCDF::NetCDF                       # creates an imported target
)

# Optionally propagate any compile definitions NetCDF needs
target_compile_definitions(netcdf
        PUBLIC
        ${NetCDF_DEFINITIONS}
)