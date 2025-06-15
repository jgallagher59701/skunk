//
// Created by James Gallagher on 5/27/25.
//

#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <string>
#include <memory>

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
class DataAccess {

public:
    DataAccess() = default;
    // explicit DataAccess(MemoryCache &cache) cache_(cache) { } jhrg later
    DataAccess(const DataAccess&) = default;
    DataAccess(DataAccess&&) noexcept = default;
    DataAccess& operator=(const DataAccess&) = default;
    DataAccess& operator=(DataAccess&&) noexcept = default;
    virtual ~DataAccess() = default;

    virtual std::string get_dmr_file(const std::string &path, const std::string &ce, const std::string &func);
    virtual std::unique_ptr<libdap::DMR> get_dmr(const std::string &path, const std::string &ce, const std::string &func) = 0;
};

#endif //DATAACCESS_H
