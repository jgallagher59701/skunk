#include "httplib.h"

int main(void) {
    httplib::Server svr;

    // Changed the first arg from /api/endpoint to / to test a matching
    // change in nginx.conf. jhrg 5/10/25
    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello from the C++ backend!\n", "text/plain");
    });

    // Listen on localhost, port 8080
    svr.listen("localhost", 8080);
}
