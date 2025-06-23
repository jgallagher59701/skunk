
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of nc_handler, a data handler for the OPeNDAP data
// server.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.


// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      reza            Reza Nekovei (reza@intcomm.net)

// This file contains functions which read the variables and their description
// from a netcdf file and build the in-memory DDS. These functions form the
// core of the server-side software necessary to extract the DDS from a
// netcdf data file.
//
// It also contains test code which will print the in-memory DDS to
// stdout.
//
// ReZa 10/20/94

// -*- mode: c++; c-basic-offset:4 -*-

// Fully modernized C++14 version of ncdds.cc with NETCDF_VERSION >= 4
// Uses RAII, constexpr, std::array, std::vector, std::string, std::unique_ptr, auto, nullptr

#include "config_nc.h"
#include <netcdf.h>
#include <libdap/DDS.h>
#include <libdap/util.h>
#include <libdap/mime_util.h>

#include "NCInt32.h"
#include "NCUInt32.h"
#include "NCInt16.h"
#include "NCUInt16.h"
#include "NCFloat64.h"
#include "NCFloat32.h"
#include "NCByte.h"
#include "NCArray.h"
#include "NCGrid.h"
#include "NCStr.h"
#include "NCStructure.h"

#include "DataAccessNetCDF.h"
#include "nc_util.h"

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <algorithm>  // for std::find

using namespace libdap;

// Compile-time constants
constexpr int MAX_VARIABLE_DIMS = MAX_NC_DIMS;
constexpr int MAX_NAME_LEN = NC_MAX_NAME + 1;

// RAII wrapper for netCDF file handle
class NetCDFFile {
    int id_ = -1;
  public:
    explicit NetCDFFile(const std::string& path) {
        if (nc_open(path.c_str(), NC_NOWRITE, &id_) != NC_NOERR) {
            throw Error("Could not open " + path + ".");
        }
    }
    ~NetCDFFile() noexcept { nc_close(id_); }
    int id() const noexcept { return id_; }
};

// Modern build_scalar returning unique_ptr
inline std::unique_ptr<BaseType> build_scalar(const std::string& name,
                                               const std::string& dataset,
                                               nc_type type) {
    switch (type) {
        case NC_STRING:
        case NC_CHAR:      return std::make_unique<NCStr>(name, dataset);
        case NC_BYTE:      return DataAccessNetCDF::get_promote_byte_to_short()
                                ? std::make_unique<NCInt16>(name, dataset)
                                : std::make_unique<NCByte>(name, dataset);
        case NC_UBYTE:     return std::make_unique<NCByte>(name, dataset);
        case NC_SHORT:     return std::make_unique<NCInt16>(name, dataset);
        case NC_USHORT:    return std::make_unique<NCUInt16>(name, dataset);
        case NC_INT:       return std::make_unique<NCInt32>(name, dataset);
        case NC_UINT:      return std::make_unique<NCUInt32>(name, dataset);
        case NC_FLOAT:     return std::make_unique<NCFloat32>(name, dataset);
        case NC_DOUBLE:    return std::make_unique<NCFloat64>(name, dataset);
        case NC_INT64:
        case NC_UINT64:
            if (DataAccessNetCDF::get_ignore_unknown_types()) {
                std::cerr << "64-bit types not supported, skipping\n";
                return nullptr;
            }
            // fall-through to error
        default:
            throw InternalErr(__FILE__, __LINE__,
                "Unsupported netCDF type " + std::to_string(type) + " for variable '" + name + "'.");
    }
}

// Modern build_grid returning unique_ptr<Grid>
inline std::unique_ptr<Grid> build_grid(std::unique_ptr<Array> ar,
                                         int ndims,
                                         nc_type type,
                                         const std::vector<std::string>& map_names,
                                         const std::vector<nc_type>& map_types,
                                         const std::vector<size_t>& map_sizes) {
    if (type == NC_CHAR) --ndims;
    for (int i = 0; i < ndims; ++i) {
        ar->append_dim(map_sizes[i], map_names[i]);
    }
    auto grid = std::make_unique<NCGrid>(ar->name(), ar->dataset());
    grid->add_var(ar.release(), libdap::array);
    for (int i = 0; i < ndims; ++i) {
        auto bt = build_scalar(map_names[i], ar->dataset(), map_types[i]);
        auto arr = std::make_unique<NCArray>(bt->name(), ar->dataset(), bt.release());
        arr->append_dim(map_sizes[i], map_names[i]);
        grid->add_var(arr.release(), maps);
    }
    return grid;
}

// Modern build_user_defined returning unique_ptr<BaseType>
inline std::unique_ptr<BaseType> build_user_defined(int ncid,
                                                    int varid,
                                                    nc_type xtype,
                                                    const std::string& dataset,
                                                    int ndims,
                                                    const int* dim_ids) {
    size_t size = 0;
    nc_type base_type;
    size_t nfields = 0;
    int class_type = 0;
    if (nc_inq_user_type(ncid, xtype, nullptr, &size, &base_type, &nfields, &class_type) != NC_NOERR) {
        throw InternalErr(__FILE__, __LINE__,
            "Could not inquire user type " + std::to_string(xtype));
    }
    if (class_type == NC_COMPOUND) {
        std::array<char, MAX_NAME_LEN> name_buf{};
        nc_inq_varname(ncid, varid, name_buf.data());
        auto structure = std::make_unique<NCStructure>(name_buf.data(), dataset);
        for (size_t i = 0; i < nfields; ++i) {
            char field_name[MAX_NAME_LEN];
            nc_type field_type;
            int field_ndims;
            int field_dim_ids[MAX_VARIABLE_DIMS];
            nc_inq_compound_field(ncid, xtype, i,
                                  field_name, nullptr,
                                  &field_type, &field_ndims,
                                  field_dim_ids);
            auto field_bt = build_user_defined(ncid, varid, field_type, dataset, field_ndims, field_dim_ids);
            if (!field_bt) field_bt = build_scalar(field_name, dataset, field_type);
            if (field_ndims == 0 || (field_ndims == 1 && field_type == NC_CHAR)) {
                structure->add_var(field_bt.release());
            } else {
                auto arr = std::make_unique<NCArray>(field_bt->name(), dataset, field_bt.release());
                for (int d = 0; d < field_ndims; ++d) arr->append_dim(field_dim_ids[d]);
                structure->add_var(arr.release());
            }
        }
        if (ndims > 0) {
            auto arr = std::make_unique<NCArray>(name_buf.data(), dataset, structure.get());
            for (int d = 0; d < ndims; ++d) arr->append_dim(dim_ids[d]);
            return arr;
        }
        return structure;
    } else {
        throw InternalErr(__FILE__, __LINE__,
            "Unsupported user-defined type class " + std::to_string(class_type));
    }
}

// Modern find_matching_coordinate_variable
inline bool find_matching_coordinate_variable(int ncid,
                                              int varid,
                                              const std::string& dimname,
                                              size_t dim_sz,
                                              nc_type& match_type) {
    int dimid = -1;
    if (nc_inq_dimid(ncid, dimname.c_str(), &dimid) != NC_NOERR)
        return false;
    size_t length = 0;
    if (nc_inq_dimlen(ncid, dimid, &length) != NC_NOERR)
        throw Error("Could not get size for dimension " + dimname);
    if (length != dim_sz)
        return false;
    int coord_varid = -1;
    if (nc_inq_varid(ncid, dimname.c_str(), &coord_varid) != NC_NOERR)
        return false;
    if (coord_varid == varid)
        return false;
    if (nc_inq_vartype(ncid, coord_varid, &match_type) != NC_NOERR)
        throw Error("Could not get type for coordinate variable " + dimname);
    return true;
}

// Modern is_grid using find_matching_coordinate_variable
inline bool is_grid(int ncid,
                    int varid,
                    int ndims,
                    const int* dim_ids,
                    std::vector<size_t>& map_sizes,
                    std::vector<std::string>& map_names,
                    std::vector<nc_type>& map_types) {
    map_names.clear();
    for (int d = 0; d < ndims; ++d) {
        std::array<char, MAX_NAME_LEN> buf{};
        size_t dim_sz = 0;
        if (nc_inq_dim(ncid, dim_ids[d], buf.data(), &dim_sz) != NC_NOERR)
            throw Error("Could not inquire dimension at id " + std::to_string(dim_ids[d]));
        nc_type type = NC_NAT;
        if (!find_matching_coordinate_variable(ncid, varid, buf.data(), dim_sz, type))
            return false;
        map_sizes[d] = dim_sz;
        map_types[d] = type;
        map_names.emplace_back(buf.data());
    }
    return true;
}

// Modern is_dimension using std::find
inline bool is_dimension(const std::string& name, const std::vector<std::string>& maps) {
    return std::find(maps.begin(), maps.end(), name) != maps.end();
}

// Modern build_array returning a raw pointer (caller takes ownership)
// Modern build_array returning unique_ptr<NCArray>
inline std::unique_ptr<NCArray> build_array(BaseType* bt,
                                             int ncid,
                                             int varid,
                                             nc_type type,
                                             int ndims,
                                             const int* dim_ids) {
    auto ar = std::make_unique<NCArray>(bt->name(), bt->dataset(), bt);
    if (type == NC_CHAR) --ndims;
    for (int d = 0; d < ndims; ++d) {
        std::array<char, MAX_NAME_LEN> buf{};
        size_t dim_sz = 0;
        if (nc_inq_dim(ncid, dim_ids[d], buf.data(), &dim_sz) != NC_NOERR) {
            throw Error("Could not get size for dimension " + std::to_string(dim_ids[d]));
        }
        ar->append_dim(dim_sz, buf.data());
    }
    return ar;
}

void read_all_variables(DDS& dds, const std::string& filename, int ncid, int nvars) {
    std::array<char, MAX_NAME_LEN> name_buf{};
    std::vector<int> dim_ids(MAX_VARIABLE_DIMS);
    for (int varid = 0; varid < nvars; ++varid) {
        int ndims = 0;
        nc_type vtype = NC_NAT;
        if (nc_inq_var(ncid, varid, name_buf.data(), &vtype, &ndims, dim_ids.data(), nullptr) != NC_NOERR) continue;
        std::vector<size_t> map_sizes(ndims);
        std::vector<nc_type> map_types(ndims);
        std::vector<std::string> map_names;
        map_names.reserve(ndims);
        if (is_grid(ncid, varid, ndims, dim_ids.data(), map_sizes.data(), nullptr, map_types.data())) {
            for (int i = 0; i < ndims; ++i) {
                std::array<char, MAX_NAME_LEN> dim_buf{};
                nc_inq_dim(ncid, dim_ids[i], dim_buf.data(), &map_sizes[i]);
                map_names.emplace_back(dim_buf.data());
            }
            auto bt = build_scalar(name_buf.data(), filename, vtype);
            auto arr = std::make_unique<NCArray>(bt->name(), filename, bt.release());
            auto grid = build_grid(std::move(arr), ndims, vtype, map_names, map_types, map_sizes);
            dds.add_var(grid.release());
        } else {
            if (ndims == 0 || (ndims == 1 && vtype == NC_CHAR)) {
                auto bt = build_scalar(name_buf.data(), filename, vtype);
                dds.add_var_nocopy(bt.release());
            } else {
                auto bt = build_scalar(name_buf.data(), filename, vtype);
                auto arr = build_array(bt.get(), ncid, varid, vtype, ndims, dim_ids.data());
                dds.add_var_nocopy(arr.release());
            }
        }
    }
}

/** Given a reference to an instance of class DDS and a filename that refers
    to a netcdf file, read the netcdf file and extract all the dimensions of
    each of its variables. Add the variables and their dimensions to the
    instance of DDS.

    @param elide_dimension_arrays If true, don't include an array if it's
    really a dimension used by a Grid. */
void nc_read_dataset_variables(DDS &dds_table, const string &filename)
{
    ncopts = 0;
    int ncid, errstat;
    int nvars;

    errstat = nc_open(filename.c_str(), NC_NOWRITE, &ncid);
    if (errstat != NC_NOERR)
        throw Error(errstat, "Could not open " + filename + ".");

    // how many variables?
    errstat = nc_inq_nvars(ncid, &nvars);
    if (errstat != NC_NOERR)
        throw Error(errstat, "Could not inquire about netcdf file: " + path_to_filename(filename) + ".");

    // dataset name
    dds_table.set_dataset_name(name_path(filename));

    // read variables' classes
    read_all_variables(dds_table, filename, ncid, nvars);

    if (nc_close(ncid) != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__, "ncdds: Could not close the dataset!");
}


/// OLD code follows
#if 0
#include "config_nc.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>

#include <netcdf.h>

#include <libdap/DDS.h>
#include <libdap/mime_util.h>
#include <libdap/util.h>

#include "DataAccessNetCDF.h"
#include "nc_util.h"

#include "NCInt32.h"
#include "NCUInt32.h"
#include "NCInt16.h"
#include "NCUInt16.h"
#include "NCFloat64.h"
#include "NCFloat32.h"
#include "NCByte.h"
#include "NCArray.h"
#include "NCGrid.h"
#include "NCStr.h"

#include "NCStructure.h"

using namespace libdap ;

/** This function returns the appropriate DODS BaseType for the given
    netCDF data type. */
static BaseType *
build_scalar(const string &varname, const string &dataset, nc_type datatype)
{
    switch (datatype) {
#if NETCDF_VERSION >= 4
        case NC_STRING:
#endif
        case NC_CHAR:
            return (new NCStr(varname, dataset));

        case NC_BYTE:
            if (DataAccessNetCDF::get_promote_byte_to_short()) {    // get_promote_byte_to_short is always false. 6/22/25
                return (new NCInt16(varname, dataset));
            }
            else {
                return (new NCByte(varname, dataset));
            }

        case NC_SHORT:
            return (new NCInt16(varname, dataset));

       case NC_INT:
            return (new NCInt32(varname, dataset));

#if NETCDF_VERSION >= 4
        case NC_UBYTE:
            // NB: the dods_byte type is unsigned
            return (new NCByte(varname, dataset));

        case NC_USHORT:
            return (new NCUInt16(varname, dataset));

        case NC_UINT:
            return (new NCUInt32(varname, dataset));
#endif
        case NC_FLOAT:
            return (new NCFloat32(varname, dataset));

        case NC_DOUBLE:
            return (new NCFloat64(varname, dataset));

#if NETCDF_VERSION >= 4
        case NC_INT64:
        case NC_UINT64:
            if (DataAccessNetCDF::get_ignore_unknown_types())
                cerr << "The netCDF handler does not currently support 64 bit integers.";
            else
                throw Error("The netCDF handler does not currently support 64 bit integers.");
            break;
#endif

        default:
            throw InternalErr(__FILE__, __LINE__, "Unknown type (" + long_to_string(datatype) + ") for variable '" + varname + "'");
    }

    return 0;
}

/** Build a grid given that one has been found. The Grid's Array is already
    allocated and is passed in along with a number of arrays containing
    information about the dimensions of the Grid.

    Note: The dim_szs and dim_nms arrays could be removed since that information
    is already in the Array ar. */
static Grid *build_grid(Array *ar, int ndims, const nc_type array_type,
        //const char map_names[MAX_NC_VARS][MAX_NC_NAME],
        std::vector<std::array<char, MAX_NC_NAME>> &map_names,
        const nc_type map_types[MAX_NC_VARS],
        const size_t map_sizes[MAX_VARIABLE_DIMS],
        vector<string> *all_maps)
{
    // Grids of NC_CHARs are treated as Grids of strings; the outermost
    // dimension (the char vector) becomes the string.
    if (array_type == NC_CHAR)
        --ndims;

    for (int d = 0; d < ndims; ++d) {
        ar->append_dim(map_sizes[d], map_names[d].data());
        // Save the map names for latter use, which might not happen...
        all_maps->emplace_back(map_names[d].data());
    }

    const string &filename = ar->dataset();
    Grid *gr = new NCGrid(ar->name(), filename);
    gr->add_var(ar, libdap::array);

    // Build and add BaseType/Array instances for the maps
    for (int d = 0; d < ndims; ++d) {
        BaseType *local_bt = build_scalar(map_names[d].data(), filename, map_types[d]);
        NCArray *local_ar = new NCArray(local_bt->name(), filename, local_bt);
        delete local_bt;
        local_ar->append_dim(map_sizes[d], map_names[d].data());
        gr->add_var(local_ar, maps);
        delete local_ar;
    }

    return gr;
}

#if NETCDF_VERSION >= 4
/** Build an instance of a user defined type. These can be recursively
 * defined.
 */
static BaseType *build_user_defined(int ncid, int varid, nc_type xtype, const string &dataset,
        int ndims, int dim_ids[MAX_VARIABLE_DIMS])
{
    size_t size;
    nc_type base_type;
    size_t nfields;
    int class_type;
    int status = nc_inq_user_type(ncid, xtype, 0/*name*/, &size, &base_type, &nfields, &class_type);
    if (status != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__, "Could not get information about a user-defined type (" + long_to_string(status) + ").");

    switch (class_type) {
        case NC_COMPOUND: {
            char var_name[NC_MAX_NAME+1];
            nc_inq_varname(ncid, varid, var_name);

            NCStructure *ncs = new NCStructure(var_name, dataset);

            for (size_t i = 0; i < nfields; ++i) {
                char field_name[NC_MAX_NAME+1];
                nc_type field_typeid;
                int field_ndims;
                int field_sizes[MAX_NC_DIMS];
                nc_inq_compound_field(ncid, xtype, i, field_name, 0, &field_typeid, &field_ndims, &field_sizes[0]);
                BaseType *field;
                if (is_user_defined_type(ncid, field_typeid)) {
		    //is_user_defined(field_typeid)) {
                    // Odd: 'varid' here seems wrong, but works.
                    field = build_user_defined(ncid, varid, field_typeid, dataset, field_ndims, field_sizes);
                    // Child compound types become anonymous variables but DAP
                    // requires names, so use the type name.
                    char var_name[NC_MAX_NAME+1];
                    nc_inq_compound_name(ncid, field_typeid, var_name);
                    field->set_name(var_name);
                }
                else {
                    field = build_scalar(field_name, dataset, field_typeid);
                }
                // is this a scalar or an array? Note that an array of CHAR is
                // a scalar string in netcdf3.
                if (field_ndims == 0 || (field_ndims == 1 && field_typeid == NC_CHAR)) {
                    ncs->add_var(field);
                }
                else {
                    NCArray *ar = new NCArray(field_name, dataset, field);
                    for (int i = 0; i < field_ndims; ++i) {
                        ar->append_dim(field_sizes[i]);
                    }
                    ncs->add_var(ar);
                }
            }
            // Is this an array of a compound (DAP Structure)?
            if (ndims > 0) {
                NCArray *ar = new NCArray(var_name, dataset, ncs);
                for (int i = 0; i < ndims; ++i) {
                    char dimname[NC_MAX_NAME+1];
                    size_t dim_sz;
                    int errstat = nc_inq_dim(ncid, dim_ids[i], dimname, &dim_sz);
                    if (errstat != NC_NOERR) {
                        delete ar;
                        throw InternalErr(__FILE__, __LINE__, string("Failed to read dimension information for the compound variable ") + var_name);
                    }

                    ar->append_dim(dim_sz, dimname);
                }

                return ar;
            }
            else {
                return ncs;
            }

            break;
        }

        case NC_VLEN:
            if (DataAccessNetCDF::get_ignore_unknown_types()) {
                cerr << "in build_user_defined; found a vlen." << endl;
                return 0;
            }
            else
                throw Error("The netCDF handler does not yet suppor the NC_VLEN type.");
            break;

        case NC_OPAQUE: {
            vector<char> name(NC_MAX_NAME+1);
            status = nc_inq_varname(ncid, varid, name.data());
            if (status != NC_NOERR)
                throw InternalErr(__FILE__, __LINE__, "Could not get name of an opaque (" + long_to_string(status) + ").");

            NCArray *opaque = new NCArray(name.data(), dataset, new NCByte(name.data(), dataset));

            if (ndims > 0) {
                for (int i = 0; i < ndims; ++i) {
                    char dimname[NC_MAX_NAME+1];
                    size_t dim_sz;
                    int errstat = nc_inq_dim(ncid, dim_ids[i], dimname, &dim_sz);
                    if (errstat != NC_NOERR) {
                        delete opaque;
                        throw InternalErr(__FILE__, __LINE__, string("Failed to read dimension information for the compound variable ") + name.data());
                    }
                    opaque->append_dim(dim_sz, dimname);
                }
            }
            opaque->append_dim(size);
            return opaque;
            break;
        }

        case NC_ENUM: {
            nc_type base_nc_type;
            size_t base_size;
            status = nc_inq_enum(ncid, xtype, 0 /*name.data()*/, &base_nc_type, &base_size, 0/*&num_members*/);
            if (status != NC_NOERR)
                throw(InternalErr(__FILE__, __LINE__, "Could not get information about an enum(" + long_to_string(status) + ")."));

            // get the name here - we want the var name and not the type name
            vector<char> name(MAX_NC_NAME + 1);
            status = nc_inq_varname(ncid, varid, name.data());
            if (status != NC_NOERR)
                throw InternalErr(__FILE__, __LINE__, "Could not get name of an opaque (" + long_to_string(status) + ").");


            BaseType *enum_var = build_scalar(name.data(), dataset, base_nc_type);

            if (ndims > 0) {
                NCArray *ar = new NCArray(name.data(), dataset, enum_var);

                for (int i = 0; i < ndims; ++i) {
                    char dimname[NC_MAX_NAME + 1];
                    size_t dim_sz;
                    int errstat = nc_inq_dim(ncid, dim_ids[i], dimname, &dim_sz);
                    if (errstat != NC_NOERR) {
                        delete ar;
                        throw InternalErr(__FILE__, __LINE__, string("Failed to read dimension information for the compound variable ") + name.data());
                    }
                    ar->append_dim(dim_sz, dimname);
                }

                return ar;
            }
            else {
                return enum_var;
            }
            break;
        }

        default:
            throw InternalErr(__FILE__, __LINE__, "Expected one of NC_COMPOUND, NC_VLEN, NC_OPAQUE or NC_ENUM");
    }

    return 0;
}
#endif

/**  Iterate over all of the variables in the data set looking for a one
     dimensional variable whose name and size match the name and size of the
     dimension 'dimname' of variable 'var'. If one is found, return it's
     name and size. It is considered to be a Map for 'var' (i.e., a matching
     coordinate variable).

     @param ncid Used with all netCDF API calls
     @param var Variable number: Look for a map for this variable.
     @param dimname Name of the current dimension
     @param dim_sz Size of the current dimension
     @param match_type Value-result parameter, holds dimension type if match found
     @return true if a match coordinate variable (i.e., map) was found, false
     otherwise

     @note In this code I scan all the variables, maybe there's a way to look
     at (only) all the shared dimensions?
 */
static bool find_matching_coordinate_variable(int ncid, int var,
        char dimname[], size_t dim_sz, nc_type *match_type)
{
    // For netCDF, a Grid's Map must be a netCDF dimension
    int id;
    // get the id matching the name.
    int status = nc_inq_dimid(ncid, dimname, &id);
    if (status == NC_NOERR) {
        // get the length, the name was matched above
        size_t length;
        status = nc_inq_dimlen(ncid, id, &length);
        if (status != NC_NOERR) {
            string msg = "netcdf 3: could not get size for dimension ";
            msg += long_to_string(id);
            msg += " in variable ";
            msg += string(dimname);
            throw Error(msg);
        }
        if (length == dim_sz) {
            // Both the name and size match and it's a dimension, so we've
            // found our 'matching coordinate variable'. To get the type,
            // Must find the variable with the name that matches the dimension.
            int varid = -1;
            status = nc_inq_varid(ncid, dimname, &varid);
            // A variable cannot be its own coordinate variable.
            // The unlimited dimension does not correspond to a variable,
            // hence the status error is means the named thing is not a
            // coordinate; it's not an error as far as the handler is concerned.
            if (var == varid || status != NC_NOERR)
                return false;

            status = nc_inq_vartype(ncid, varid, match_type);
            if (status != NC_NOERR) {
                string msg = "netcdf 3: could not get type variable ";
                msg += string(dimname);
                throw Error(msg);
            }

            return true;
        }
    }
    return false;
}

/** Is the variable a DAP Grid?
     @param ncid The open netCDF file id
     @param nvars The number of variables in the file. Needed by the function
     that looks for maps.
     @param var The id of the variable we're asking about.
     @param ndims The number of dimensions in 'var'.
     @param dim_ids The dimension ids of 'var'.
     @param map_sizes Value-result parameter; the size of each map.
     @param map_names Value-result parameter; the name of each map.
     @param map_types Value-result parameter; the type of each map.
 */
static bool is_grid(int ncid, int var, int ndims, const int dim_ids[MAX_VARIABLE_DIMS],
        size_t map_sizes[MAX_VARIABLE_DIMS],
        //char map_names[MAX_NC_VARS][MAX_NC_NAME],
        std::vector<std::array<char, MAX_NC_NAME>> &map_names,
        nc_type map_types[MAX_NC_VARS])
{
    // Look at each dimension of the variable.
    for (int d = 0; d < ndims; ++d) {
        char dimname[MAX_NC_NAME];
        size_t dim_sz;

        int errstat = nc_inq_dim(ncid, dim_ids[d], dimname, &dim_sz);
        if (errstat != NC_NOERR) {
            string msg = "netcdf 3: could not get size for dimension ";
            msg += long_to_string(d);
            msg += " in variable ";
            msg += long_to_string(var);
            throw Error(msg);
        }

        nc_type match_type;
        if (find_matching_coordinate_variable(ncid, var, dimname, dim_sz, &match_type)) {
            map_types[d] = match_type;
            map_sizes[d] = dim_sz;
            strncpy(map_names[d].data(), dimname, MAX_NC_NAME - 1);
            map_names[d][MAX_NC_NAME - 1] = '\0';
        }
        else {
            return false;
        }
    }

    return true;
}

static bool is_dimension(const string &name, vector<string> maps)
{
    auto i = find(maps.begin(), maps.end(), name);
    if (i != maps.end())
        return true;
    else
        return false;
}

static NCArray *build_array(BaseType *bt, int ncid, int var,
        const nc_type array_type, int ndims,
        const int dim_ids[MAX_NC_DIMS])
{
    NCArray *ar = new NCArray(bt->name(), bt->dataset(), bt);

    if (array_type == NC_CHAR)
        --ndims;

    for (int d = 0; d < ndims; ++d) {
        char dimname[MAX_NC_NAME];
        size_t dim_sz;
        int errstat = nc_inq_dim(ncid, dim_ids[d], dimname, &dim_sz);
        if (errstat != NC_NOERR) {
        	delete ar;
            throw Error("netcdf: could not get size for dimension " + long_to_string(d) + " in variable " + long_to_string(var));
        }

        ar->append_dim(dim_sz, dimname);
    }

    return ar;
}

/** Read given number of variables (nvars) from the opened netCDF file
     (ncid) and add them with their appropriate type and dimensions to
     the given instance of the DDS class.

     @param dds_table Add variables to this DDS object
     @param filename When making new variables, record this as the source
     @param ncid The id of the netcdf file
     @param nvars The number of variables in the opened file
 */
static void read_variables(DDS &dds_table, const string &filename, int ncid, int nvars)
{
    // How this function works: The variables are scanned once, but because
    // netCDF includes shared dimensions as variables, there are two versions
    // of this function. One writes out the variables as they are found while
    // the other writes scalars and Grids as they are found and saves Arrays
    // for output last. When writing the arrays, it checks to see if
    // an array variable is also a grid dimension and, if so, does not write
    // it out. Thus, in the second version of the function, all arrays appear
    // after the other variable types and only those arrays that do not
    // appear as Grid Maps are included.

    // These two vectors are used to record the ids of array variables and
    // the names of all the Grid Map variables
    vector<int> array_vars;
    vector<string> all_maps;

    // These are defined here since they are used by both loops.
    char name[MAX_NC_NAME];
    nc_type nctype;
    int ndims;
    int dim_ids[MAX_VARIABLE_DIMS];

    // Examine each variable in the file; if 'elide_grid_maps' is true, adds
    // only scalars and Grids (Arrays are added in the following loop). If
    // false, all variables are added in this loop.
    for (int varid = 0; varid < nvars; ++varid) {
        int errstat = nc_inq_var(ncid, varid, name, &nctype, &ndims, dim_ids, (int *) 0);
        if (errstat != NC_NOERR)
            throw Error("netcdf: could not get name or dimension number for variable " + long_to_string(varid));

        // These are defined here because they are value-result parameters for
        // is_grid() called below.
        size_t map_sizes[MAX_VARIABLE_DIMS];
        // char map_names[MAX_NC_VARS][MAX_NC_NAME];
        std::vector<std::array<char, MAX_NC_NAME>> map_names(ndims);
        nc_type map_types[MAX_NC_VARS];

        // a scalar? NB a one-dim NC_CHAR array will have DAP type of
        // dods_str_c because it's really a scalar string, not an array.
        if (is_user_defined_type(ncid, nctype)) {
            BaseType *bt = build_user_defined(ncid, varid, nctype, filename, ndims, dim_ids);
            dds_table.add_var(bt);
            delete bt;
        }
        else if (ndims == 0 || (ndims == 1 && nctype == NC_CHAR)) {
            BaseType *bt = build_scalar(name, filename, nctype);
            dds_table.add_var(bt);
            delete bt;
        }
        else if (is_grid(ncid, varid, ndims, dim_ids, map_sizes, map_names, map_types)) {
            BaseType *bt = build_scalar(name, filename, nctype);
            Array *ar = new NCArray(name, filename, bt);
            delete bt;
            Grid *gr = build_grid(ar, ndims, nctype, map_names, map_types, map_sizes, &all_maps);
            delete ar;
            dds_table.add_var(gr);
            delete gr;
        }
        else {
            if (!DataAccessNetCDF::get_show_shared_dims()) { // get_show_shared_dims is now false by default. 6/22/25
                array_vars.push_back(varid);
            } else {
                BaseType *bt = build_scalar(name, filename, nctype);
                NCArray *ar = build_array(bt, ncid, varid, nctype, ndims, dim_ids);
                delete bt;
                dds_table.add_var(ar);
                delete ar;
            }
        }
    }

    // This code is only run if elide_dimension_arrays is true and in that case the
    // loop above did not create any simple arrays. Instead it pushed the
    // var ids of things that look like simple arrays onto a vector. This code
    // will add all of those that really are arrays and not the ones that are
    // dimensions used by a Grid.
    if (!DataAccessNetCDF::get_show_shared_dims()) { // get_show_shared_dims is now false by default. 6/22/25
        // Now just loop through the saved array variables, writing out only
        // those that are not Grid Maps
        nvars = array_vars.size();
        for (int i = 0; i < nvars; ++i) {
            int var = array_vars.at(i);

            int errstat = nc_inq_var(ncid, var, name, &nctype, &ndims,
                    dim_ids, (int *) 0);
            if (errstat != NC_NOERR) {
                string msg = "netcdf 3: could not get name or dimension number for variable ";
                msg += long_to_string(var);
                throw Error(msg);
            }

            // If an array already appears as a Grid Map, don't write it out
            // as an array too.
            if (is_dimension(string(name), all_maps))
                continue;

            BaseType *bt = build_scalar(name, filename, nctype);
            Array *ar = build_array(bt, ncid, var, nctype, ndims, dim_ids);
            delete bt;
            dds_table.add_var(ar);
            delete ar;
        }
    }
}
#endif
#if 0
  /** Given a reference to an instance of class DDS and a filename that refers
    to a netcdf file, read the netcdf file and extract all the dimensions of
    each of its variables. Add the variables and their dimensions to the
    instance of DDS.

    @param elide_dimension_arrays If true, don't include an array if it's
    really a dimension used by a Grid. */
void nc_read_dataset_variables(DDS &dds_table, const string &filename)
{
    ncopts = 0;
    int ncid, errstat;
    int nvars;

    errstat = nc_open(filename.c_str(), NC_NOWRITE, &ncid);
    if (errstat != NC_NOERR)
        throw Error(errstat, "Could not open " + filename + ".");

    // how many variables?
    errstat = nc_inq_nvars(ncid, &nvars);
    if (errstat != NC_NOERR)
        throw Error(errstat, "Could not inquire about netcdf file: " + path_to_filename(filename) + ".");

    // dataset name
    dds_table.set_dataset_name(name_path(filename));

    // read variables' classes
    read_variables(dds_table, filename, ncid, nvars);

    if (nc_close(ncid) != NC_NOERR)
        throw InternalErr(__FILE__, __LINE__, "ncdds: Could not close the dataset!");
}
#endif


