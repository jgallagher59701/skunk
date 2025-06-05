//
// Created by James Gallagher on 5/26/25.
//

#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>

#include "httplib.h"

using namespace std;

int main(void) {
    httplib::Server svr;

    // Changed the first arg from /api/endpoint to / to test a matching
    // change in nginx.conf. jhrg 5/10/25
    svr.Get(R"(/.+)", [](const httplib::Request &req, httplib::Response &res) {
        auto headers = req.headers;
        auto path = req.path;
        //auto  = req.matches[1];
        auto path_params = req.path_params;

        ostringstream oss;
        oss << "Request headers: \n";
        std::for_each(headers.begin(), headers.end(), [&oss](const auto& kv) {
            oss << "    " << kv.first << ": " << kv.second << "\n";
        });
        oss << "Path: " << path << "\n";
        oss << "Path parameters: \n" ;
        auto ce = req.get_param_value("ce");
        auto function = req.get_param_value("function");
        oss << "    ce: " << ce << "\n";
        oss << "    function: " << function << "\n";
#if 0
        std::for_each(path_params.begin(), path_params.end(), [&oss](const auto& kv) {
            oss << "    " << kv.first << ": " << kv.second << "\n";
        });
#endif

        res.set_content(oss.str(), "text/plain");
    });

    // Listen on localhost, port 8080
    svr.listen("localhost", 8080);
}
