//
// Created by James Gallagher on 5/27/25.
//

#ifndef DATAACCESSNETCF_H
#define DATAACCESSNETCF_H

#include <string>
#include <memory>

#include <libdap/DMR.h>

#include "DataAccess.h"

namespace libdap {
class DMR;
}

/**
 * @brief Subclass this for different types of data sources
 *
 * This class provides an interface that builds a DAP4 DMR for the data source
 * referenced by a path, subject to a DAP4 constraint expression and server
 * side function. The getDMR() method should be specialized for different
 * data types (e.g., netCDF4 files).
 *
 * The base class holds a reference to a MemoryCache instance that can be used
 * to cache binary DMR objects as well as other things.
 */
class DataAccessNetCDF: public DataAccess {

public:
    DataAccessNetCDF() = default;
    // explicit DataAccessNetCDF(MemoryCache &cache) cache_(cache) { } jhrg later
    DataAccessNetCDF(const DataAccessNetCDF&) = default;
    DataAccessNetCDF(DataAccessNetCDF&&) noexcept = default;
    DataAccessNetCDF& operator=(const DataAccessNetCDF&) = default;
    DataAccessNetCDF& operator=(DataAccessNetCDF&&) noexcept = default;
    ~DataAccessNetCDF() override = default;

#if 0

    virtual std::unique_ptr<libdap::DMR> get_dmr(const std::string &path, const std::string &ce, const std::string &func);
    virtual std::unique_ptr<libdap::DMR> get_dmr(const std::string &path, const std::string &ce, const std::string &func) {
        return make_unique<libdap::DMR>()
    }
    virtual std::unique_ptr<libdap::DMR> get_dmr(const std::string &path, const std::string &ce) = 0;
    virtual std::unique_ptr<libdap::DMR> get_dmr(const std::string &path) = 0;

#endif
};

#endif //DATAACCESSNETCF_H
