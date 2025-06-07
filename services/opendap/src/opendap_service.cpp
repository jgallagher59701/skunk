//
// Created by James Gallagher on 5/26/25.
//

#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>

#include "handler.h"
// #include "DataAccess"


using namespace std;

std::string format_extension(const std::string& filename) {
    size_t last_dot_pos = filename.rfind('.');
    if (last_dot_pos == std::string::npos) {
        // No dot found, return the original string
        return filename;
    }
    // Find the last separator (e.g., '/' or '\') to handle paths
    size_t last_separator_pos = filename.find_last_of('/');
    if (last_separator_pos != std::string::npos && last_dot_pos < last_separator_pos) {
        // Dot is part of the path, not an extension
        return filename;
    }
    return filename.substr(last_dot_pos);
}

#if 0
// test strings for the above
std::string filename1 = "example.txt";
  std::string filename2 = "archive.tar.gz";
  std::string filename3 = "no_extension";
  std::string filename4 = "path/to/file.name.ext";
  std::string filename5 = "path.with.dots/file";
#endif

enum data_format {
    nc,
    unknown
};

/**
 * @brief Assume formats are determined by extension.
 * @param data_path
 * @return
 */
data_format find_format(const string &data_path) {
    auto format = format_extension(data_path);
    if (format == ".nc")
        return nc;
    return unknown;
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

int main() {
    httplib::Server svr;

    // Register loger - see logger in example
    // Register exception handler

    // The DMR return
    svr.Get(R"(/(.+)\.dmr)", [](const httplib::Request& req, httplib::Response& res) {
        // Extract parameters
        if (req.matches.empty()) {
            res.set_content("Error - no data path found", "text/plain");
        }
        const auto data_path = req.matches[1];
        const auto ce = req.has_param("dap4_ce") ? req.get_param_value("dap4_ce") : "";
        const auto function = req.has_param("dap4_function") ? req.get_param_value("dap4_function") : "";

        // Determine data format
        const auto format = find_format(data_path);

        // Output
        ostringstream oss;
        oss << "data_path: " << data_path << '\n';
        oss << "dap4_ce: " << ce << '\n';
        oss << "dap4_function: " << function << '\n';

        if (format == nc)
            oss << "format: nc\n";
        else
            oss << "format: unknown\n";

        // Get the DataAccess instance
        // DataAccess da = find_data_access(format);
        // Get the DMR
        // Build the DMR response
        // Return the DMR response
        res.set_content(oss.str(), "text/plain");
    });

    svr.listen("localhost", 8080);
}
