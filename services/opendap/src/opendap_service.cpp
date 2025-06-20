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
    svr.Get(R"(/opendap/(.+)\.dmr)", [](const httplib::Request& req, httplib::Response& res) {
        // Extract parameters
        if (req.matches.empty()) {
            res.set_content("Error - no data path found", "text/plain");
        }
        const auto data_path = req.matches[1];

        hyrax::handle_dmr_request(data_path, req, res);
    });

    svr.listen("localhost", 8080);
}
