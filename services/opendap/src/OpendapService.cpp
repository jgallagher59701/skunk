//
// Created by James Gallagher on 5/26/25.
//

#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>

#include "handler.h"

using namespace std;

int main(void) {
    httplib::Server svr;

    svr.Get(R"(/.+)", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(handle_request(req), "text/plain");
    });

    svr.listen("localhost", 8080);
}
