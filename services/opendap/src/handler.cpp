//
// Created by James Gallagher on 6/5/25.
//

#include <string>
#include <sys/stat.h>
#include <ctime>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include "handler.h"
#include "DataAccessNetCDF.h"

using namespace std;

string get_extension(const string& filename) {
    // Find the last path separator
    const size_t last_slash_pos = filename.find_last_of('/');

    // Isolate the last path component (the file name)
    const string basename = (last_slash_pos != string::npos)
                      ? filename.substr(last_slash_pos + 1)
                      : filename;

    // Look for the last dot in the basename
    const size_t last_dot_pos = basename.rfind('.');
    if (last_dot_pos == string::npos) {
        // No extension in the filename part → return original input
        return filename;
    }

    // Otherwise, return the extension
    return basename.substr(last_dot_pos);
}

/**
 * @brief Assume formats are determined by extension.
 * @param data_path
 * @return
 */
data_format find_format(const string &data_path) {
    auto format = get_extension(data_path);
    if (format == ".nc")
        return nc;
    return unknown_format;
}

//Date: Tue, 17 Jun 2025 01:16:00 GMT
//Server: Apache/2.4.56 (Unix)
//X-FRAME-OPTIONS: DENY
//Last-Modified: Thu, 23 Sep 2021 19:45:14 GMT
//XDODS-Server: dods/3.2
//XOPeNDAP-Server: asciival/, bes/, builddmrpp_module/, csv_handler/, dapreader_module/, dmrpp_module/, fileout_covjson/, fileout_json/, fileout_netcdf/, freeform_handler/, functions/, gateway/, gdal_module/, hdf4_handler/, hdf5_handler/, libdap/, ncml_moddule/, netcdf_handler/, ngap_module/, s3_reader/, usage/, w10n_handler/, xml_data_handler/
//X-DAP: 3.2
//Content-Description: application/vnd.opendap.dap4.dataset-metadata+xml
//Content-Type: application/vnd.opendap.dap4.dataset-metadata+xml
//
// nginx adds: Server, Date, Connection,
// and Content-Length (modifies this, I think, because cpp-httplib sets it.)

void set_dmr_response_headers(httplib::Response& res, const string &date) {
    res.set_header("Cache-Control", "public");
    res.set_header("Last-Modified", date);
    res.set_header("X-FRAME-OPTIONS", "DENY");
    res.set_header("XDODS-Server", "dods/3.2");
    res.set_header("X-DAP", "4.0");
    res.set_header("XOPeNDAP-Server", "s-works");
    res.set_header("Content-Description", "application/vnd.opendap.dap4.dataset-metadata+xml");
    res.set_header("Content-Type", "application/vnd.opendap.dap4.dataset-metadata+xml");
}

time_t get_last_modification_time(const string& filepath) {
    struct stat file_info{};
    if (stat(filepath.c_str(), &file_info) != 0) {
        throw runtime_error("Unable to retrieve file info: " + filepath);
    }
    return file_info.st_mtime;
}

string format_http_date(time_t t) {
    char buf[100];
    tm tm{};
    gmtime_r(&t, &tm);  // Thread-safe UTC conversion
    if (strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm) == 0) {
        throw runtime_error("strftime failed");
    }
    return {buf};
}
#if 0
unique_ptr<DataAccess>  find_data_access(const enum data_format &df) {
    switch (df) {
        case nc:
            return make_unique<DataAccessNetCDF>();
        case unknown:
            throw std::runtime_error("Unknown data format."0;)
    }
}
#endif

void handle_dmr_request(const string &data_path, const httplib::Request& req, httplib::Response& res) {
    // extract info from headers, etc.
    const auto ce = req.has_param("dap4_ce") ? req.get_param_value("dap4_ce") : "";
    const auto function = req.has_param("dap4_function") ? req.get_param_value("dap4_function") : "";

    const auto format = find_format(data_path);
    const string data_root = "/Users/jimg/src/opendap/skunk/services/opendap/src/"; // FIXME jhrg 6/16/25
    if (format == nc) {
        DataAccessNetCDF format_handler;
        if (data_path.find("fnoc1.nc") != string::npos) {
            const auto full_data_path = data_root + data_path + ".dmr";
            const auto lmt_date = format_http_date(get_last_modification_time(full_data_path));
            set_dmr_response_headers(res, lmt_date);
            res.set_content(format_handler.get_dmr_file(data_root + data_path + ".dmr"),
                            "application/vnd.opendap.dap4.dataset-metadata+xml");
            return;
        }
#if 0
        auto dmr = format_handler.get_dmr("Moof!");
        ostringstream oss;
        dmr->print_dap4(oss);
#endif
    	res.set_content("Moof!", "text/plain");
		return;
	}

    res.set_content("Error: only netCDF files can be served.", "text/plain");
}
