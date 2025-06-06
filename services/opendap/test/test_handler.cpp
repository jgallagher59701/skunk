//
// Created by James Gallagher on 6/5/25.
//

#include "handler.h"
#include <gtest/gtest.h>

TEST(HandlerTest, HandlesRequestWithHeadersAndParams) {
httplib::Request req;
req.path = "/api/test";
req.headers = {
        {"X-Test-Header", "test-value"},
        {"User-Agent", "gtest-client"}
};
req.params = {
        {"ce", "123"},
        {"function", "echo"}
};
req.path_params = {
        {"id", "42"}
};

std::string output = handle_request(req);

EXPECT_NE(output.find("X-Test-Header: test-value"), std::string::npos);
EXPECT_NE(output.find("User-Agent: gtest-client"), std::string::npos);
EXPECT_NE(output.find("Path: /api/test"), std::string::npos);
EXPECT_NE(output.find("ce: 123"), std::string::npos);
EXPECT_NE(output.find("function: echo"), std::string::npos);
EXPECT_NE(output.find("id: 42"), std::string::npos);
}
