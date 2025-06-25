
add_library(netcdf_support STATIC)

target_sources(netcdf_support PRIVATE
        nc_util.cc		ncdas.cc        new_ncdds.cc
        NCArray.cc		NCGrid.cc		NCStr.cc
        NCByte.cc		NCInt16.cc		NCStructure.cc
        NCInt32.cc		NCUInt16.cc     NCUInt32.cc
        NCFloat32.cc    NCFloat64.cc	NCSequence.cc
        NCUrl.cc        DataAccessNetCDF.cpp
)

# NC_INCLUDE_DIR and NC_LIB arem set in the top-level
# CMakeLists file. jhrg 6/21/25

target_include_directories(netcdf_support PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OPENDAP_SRC}
        ${NC_INCLUDE_DIR}               # NetCDF::NetCDF provides this
)

# Link against the NetCDF library
target_link_libraries(netcdf_support
        PUBLIC
        ${NC_LIB}                       # creates an imported target
)

# Optionally propagate any compile definitions NetCDF needs
target_compile_definitions(netcdf_support
        PUBLIC
        ${NetCDF_DEFINITIONS}
)