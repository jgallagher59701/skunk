//
// Created by James Gallagher on 6/5/25.
//

#include <string>
#include <sstream>

#include "handler.h"

using namespace std;

string get_extension(const string& filename) {
    // Find the last path separator
    size_t last_slash_pos = filename.find_last_of('/');

    // Isolate the last path component (the file name)
    string basename = (last_slash_pos != string::npos)
                      ? filename.substr(last_slash_pos + 1)
                      : filename;

    // Look for the last dot in the basename
    size_t last_dot_pos = basename.rfind('.');
    if (last_dot_pos == string::npos) {
        // No extension in the filename part → return original input
        return filename;
    }

    // Otherwise, return the extension
    return basename.substr(last_dot_pos);
}

#if 0
/**
 * @brief Assume responses are determined by extension
 * Use an extension on the path to preserve compatibility with existing clients.
 * @param data_path
 * @return dap_response enum
 */
dap_response find_response(const string &data_path) {
    auto response = get_extension(data_path);
    if (response == ".dmr")
        return dmr;
    else if (response == ".dap")
        return dap;

    return unknown_response;
}
#endif

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

    if (format == nc) {
        res.set_content("Error: only netCDF files can be served.", "text/plain");
        return;
    }

    // send response headers, blank line
    // send response body
    res.set_content("Moof!", "text/plain");
}
