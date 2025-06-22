//
// Created by James Gallagher on 5/27/25.
//

#include <memory>

#include <libdap/DMR.h>
#include <libdap/DDS.h>
#include <libdap/DAS.h>
#include <libdap/Ancillary.h>
#include <libdap/D4BaseTypeFactory.h>

#include "DataAccessNetCDF.h"

using namespace std;
using namespace libdap;

bool DataAccessNetCDF::show_shared_dims_ = false;   // Originally true, for DAP2. 6/22/25
bool DataAccessNetCDF::ignore_unknown_types_ = false;
bool DataAccessNetCDF::promote_byte_to_short_ = false;

// These are defined in ncdds.cc and ncdas.cc. 6/22/25
extern void nc_read_dataset_attributes(DAS & das, const string & filename);
extern void nc_read_dataset_variables(DDS & dds, const string & filename);

std::unique_ptr<DMR> DataAccessNetCDF::get_dmr(const std::string &path, const std::string &ce, const std::string &func) {

    // Build the DMR using the DDS and DAS. Improve this later.
    // The original netCDF handler software did not use a formal factory class,
    // so this code uses the generic factory class.
    BaseTypeFactory factory;
    DDS dds(&factory, "", "4.0");   // The name will be set by nc_read_dataset_variables(). 6/22/25
    nc_read_dataset_variables(dds, path);

    DAS das;
    nc_read_dataset_attributes(das, path);
    Ancillary::read_ancillary_das(das, path);

    dds.transfer_attributes(&das);

    auto dmr= make_unique<libdap::DMR>();

    D4BaseTypeFactory dap4_factory;
    dmr->set_factory(&dap4_factory);
    dmr->build_using_dds(dds);
    dmr->set_factory(nullptr);
    return dmr;
}

