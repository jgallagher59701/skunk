//
// Created by James Gallagher on 6/5/25.
//

#include <string>
#include <sstream>

#include "handler.h"

std::string handle_request(const httplib::Request& req) {
    std::ostringstream oss;

    oss << "Request headers: \n";
    for (const auto& kv : req.headers) {
        oss << "    " << kv.first << ": " << kv.second << "\n";
    }

    oss << "Path: " << req.path << "\n";
    oss << "Path parameters: \n";
    oss << "    ce: " << req.get_param_value("ce") << "\n";
    oss << "    function: " << req.get_param_value("function") << "\n";

    for (const auto& kv : req.path_params) {
        oss << "    " << kv.first << ": " << kv.second << "\n";
    }

    return oss.str();
}
