//
// Created by James Gallagher on 5/27/25.
//

#include <memory>
#include <libdap/DMR.h>

#include "DataAccessNetCDF.h"

using namespace std;

std::unique_ptr<libdap::DMR>
DataAccessNetCDF::getDMR(const std::string &path, const std::string &ce, const std::string &func) {
    auto dmr = make_unique<libdap::DMR>();
    return dmr;
}