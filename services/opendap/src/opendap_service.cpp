//
// Created by James Gallagher on 5/26/25.
//

#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>

#include "handler.h"

using namespace std;


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

        handle_dmr_request(data_path, req, res);
#if 0
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
#endif
    });

    svr.listen("localhost", 8080);
}
