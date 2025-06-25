
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
//      Reza Nekovei (reza@intcomm.net)

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

#include "nc_util.h"
#include "DataAccessNetCDF.h"

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
    explicit NetCDFFile(const std::string &path)
    {
        if (nc_open(path.c_str(), NC_NOWRITE, &id_) != NC_NOERR) {
            throw Error("Could not open " + path + ".");
        }
    }

    ~NetCDFFile() noexcept
    { nc_close(id_); }

    int id() const noexcept
    { return id_; }
};

/**
 * @brief Factory for scalar netcdf types
 * @param name The variable name
 * @param dataset The dataset name
 * @param type The netcdf type (a nc_type value)
 * @return A unique_ptr<BaseType> to the instance
 */
inline std::unique_ptr<BaseType> build_scalar(const std::string &name, const std::string &dataset, nc_type type)
{
    switch (type) {
        case NC_STRING:
        case NC_CHAR:
            return std::make_unique<NCStr>(name, dataset);
        case NC_BYTE:
            if (DataAccessNetCDF::get_promote_byte_to_short())
                return std::make_unique<NCInt16>(name, dataset);
            else
                return std::make_unique<NCByte>(name, dataset);
       case NC_UBYTE:
            return std::make_unique<NCByte>(name, dataset);
        case NC_SHORT:
            return std::make_unique<NCInt16>(name, dataset);
        case NC_USHORT:
            return std::make_unique<NCUInt16>(name, dataset);
        case NC_INT:
            return std::make_unique<NCInt32>(name, dataset);
        case NC_UINT:
            return std::make_unique<NCUInt32>(name, dataset);
        case NC_FLOAT:
            return std::make_unique<NCFloat32>(name, dataset);
        case NC_DOUBLE:
            return std::make_unique<NCFloat64>(name, dataset);
        case NC_INT64:
        case NC_UINT64:
            // TODO Add support for 64-bit ints. 6/23/25 jhrg
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

/**
 * @brief Build a DAP2 Grid using an array and a collection of 'maps.'
 * @param ar
 * @param ndims
 * @param type
 * @param map_names
 * @param map_types
 * @param map_sizes
 * @return A unique_ptr<Grid> instance.
 */
inline std::unique_ptr<Grid> build_grid(std::unique_ptr<Array> ar, int ndims, nc_type type,
                                        const std::vector<std::string> &map_names, const std::vector<nc_type> &map_types,
                                        const std::vector<size_t> &map_sizes)
{
    if (type == NC_CHAR) --ndims;
    for (int i = 0; i < ndims; ++i) {
        ar->append_dim(map_sizes[i], map_names[i]);
    }
    auto grid = std::make_unique<NCGrid>(ar->name(), ar->dataset());

    for (int i = 0; i < ndims; ++i) {
        auto bt = build_scalar(map_names[i], ar->dataset(), map_types[i]);
        auto arr = std::make_unique<NCArray>(bt->name(), ar->dataset(), bt.release());
        arr->append_dim(map_sizes[i], map_names[i]);
        grid->add_var_nocopy(arr.release(), maps);
    }
    grid->add_var_nocopy(ar.release(), libdap::array);
    return grid;
}

// Modern build_user_defined returning unique_ptr<BaseType>
inline std::unique_ptr<BaseType> build_user_defined(int ncid, int varid, nc_type xtype, const std::string &dataset,
                                                    int ndims, std::vector<int> dim_ids)
{
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
            std::vector<int> field_dim_ids(MAX_VARIABLE_DIMS);
            nc_inq_compound_field(ncid, xtype, i,
                                  field_name, nullptr,
                                  &field_type, &field_ndims,
                                  field_dim_ids.data());
            auto field_bt = build_user_defined(ncid, varid, field_type, dataset, field_ndims, field_dim_ids);
            if (!field_bt) field_bt = build_scalar(field_name, dataset, field_type);
            if (field_ndims == 0 || (field_ndims == 1 && field_type == NC_CHAR)) {
                structure->add_var(field_bt.release());
            }
            else {
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
    }
    else {
        throw InternalErr(__FILE__, __LINE__,
                          "Unsupported user-defined type class " + std::to_string(class_type));
    }
}

// Modern find_matching_coordinate_variable
inline bool find_matching_coordinate_variable(int ncid, int varid, const std::string &dimname, size_t dim_sz,
                                              nc_type &match_type)
{
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
inline bool is_grid(int ncid, int varid, int ndims, std::vector<int> &dim_ids, std::vector<size_t> &map_sizes,
                    std::vector<std::string> &map_names, std::vector<nc_type> &map_types)
{
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
inline bool is_dimension(const std::string &name, const std::vector<std::string> &maps)
{
    return std::find(maps.begin(), maps.end(), name) != maps.end();
}

inline std::unique_ptr<NCArray> build_array(BaseType *bt, int ncid, int varid, nc_type type, int ndims, const int *dim_ids)
{
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

void read_all_variables(DDS &dds, const std::string &filename, int ncid, int nvars)
{
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
        if (is_grid(ncid, varid, ndims, dim_ids, map_sizes, map_names, map_types)) {
            for (int i = 0; i < ndims; ++i) {
                std::array<char, MAX_NAME_LEN> dim_buf{};
                nc_inq_dim(ncid, dim_ids[i], dim_buf.data(), &map_sizes[i]);
                map_names.emplace_back(dim_buf.data());
            }
            auto bt = build_scalar(name_buf.data(), filename, vtype);
            auto arr = std::make_unique<NCArray>(bt->name(), filename, bt.release());
            auto grid = build_grid(std::move(arr), ndims, vtype, map_names, map_types, map_sizes);
            dds.add_var_nocopy(grid.release());
        }
        else if (is_user_defined_type(ncid, vtype)) {
            auto bt = build_user_defined(ncid, varid, vtype, filename, ndims, dim_ids);
            dds.add_var_nocopy(bt.release());
        }
        else if (ndims == 0 || (ndims == 1 && vtype == NC_CHAR)) {
            auto bt = build_scalar(name_buf.data(), filename, vtype);
            dds.add_var_nocopy(bt.release());
        }
        else {
            auto bt = build_scalar(name_buf.data(), filename, vtype);
            auto arr = build_array(bt.get(), ncid, varid, vtype, ndims, dim_ids.data());
            dds.add_var_nocopy(arr.release());
        }
    }
}

/**
 * @brief Build a DDS for a netCDF file
 *
 * Given a reference to an instance of class DDS and a filename that refers
 * to a netcdf file, read the netcdf file and extract all the variables and
 * their dimensions. Add the variables and their dimensions to the instance of DDS.
 *
 * @param dds_table Add information to this DDS instance
 * @param filename The path to the netCDF file.
 */
void nc_read_dataset_variables(DDS &dds_table, const string &filename)
{
    const NetCDFFile nc_file(filename);

    // dataset name
    dds_table.set_dataset_name(name_path(filename));

    // how many variables?
    int nvars;
    auto errstat = nc_inq_nvars(nc_file.id(), &nvars);
    if (errstat != NC_NOERR)
        throw Error(errstat, "Could not inquire about netcdf file: " + path_to_filename(filename) + ".");

    // read variables' classes
    read_all_variables(dds_table, filename, nc_file.id(), nvars);
}


