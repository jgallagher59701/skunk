//
// Created by James Gallagher on 5/27/25.
//

#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <string>
#include <memory>

#include "MemoryCache.h"

namespace libdap {
class DMR;

}


class DataAccess {
    /// Add an instance of MemoryCache here.
    MemoryCache &cache_;

public:
    DataAccess() = delete;
    explicit DataAccess(MemoryCache &cache) cache_(cache) { }
    DataAccess(const DataAccess&) = delete;
    DataAccess(DataAccess&&) noexcept = delete;
    DataAccess& operator=(const DataAccess&) = delete;
    DataAccess& operator=(DataAccess&&) noexcept = delete;
    virtual ~DataAccess() = default;

    virtual std::unique_ptr<DMR> getDMR(const std::string &path, const std::string &ce, const std::string &func) = 0;

};



#endif //DATAACCESS_H
