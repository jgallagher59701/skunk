//
// Created by James Gallagher on 6/5/25.
//

#ifndef SKUNK_HANDLER_H
#define SKUNK_HANDLER_H

#include <string>

#include "httplib.h"

enum data_format {
    nc,
    unknown_format
};

#if 0
enum dap_response {
    dap,
    dmr,
    unknown_response
};
#endif

void handle_dmr_request(const std::string &data_path, const httplib::Request& req, httplib::Response& res);

std::string get_extension(const std::string& filename);
data_format find_format(const std::string &data_path);
void set_dmr_response_headers(httplib::Response& res, const std::string &date);
time_t get_last_modification_time(const std::string& filepath);
std::string format_http_date(time_t t);

#if 0
dap_response find_response(const std::string &data_path);
#endif

#endif //SKUNK_HANDLER_H
