//
// Created by James Gallagher on 6/5/25.
//

#include "handler.h"
#include <gtest/gtest.h>

#if 0
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

    std::string output = handle_dmr_request(req);

    EXPECT_NE(output.find("X-Test-Header: test-value"), std::string::npos);
    EXPECT_NE(output.find("User-Agent: gtest-client"), std::string::npos);
    EXPECT_NE(output.find("Path: /api/test"), std::string::npos);
    EXPECT_NE(output.find("ce: 123"), std::string::npos);
    EXPECT_NE(output.find("function: echo"), std::string::npos);
    EXPECT_NE(output.find("id: 42"), std::string::npos);
}

#endif

TEST(FormatExtensionTest, HandlesNormalExtensions) {
    EXPECT_EQ(get_extension("file.txt"), ".txt");
    EXPECT_EQ(get_extension("archive.tar.gz"), ".gz");
}

TEST(FormatExtensionTest, HandlesNoExtension) {
    EXPECT_EQ(get_extension("Makefile"), "Makefile");
    EXPECT_EQ(get_extension("README"), "README");
}

TEST(FormatExtensionTest, HandlesHiddenFiles) {
    EXPECT_EQ(get_extension(".bashrc"), ".bashrc");
    EXPECT_EQ(get_extension(".env.local"), ".local");
}

TEST(FormatExtensionTest, HandlesPathWithExtension) {
    EXPECT_EQ(get_extension("/home/user/file.txt"), ".txt");
    EXPECT_EQ(get_extension("/home/user/archive.tar.gz"), ".gz");
}

TEST(FormatExtensionTest, HandlesDotInDirectoryName) {
    EXPECT_EQ(get_extension("/home/user/.config/file"), "/home/user/.config/file");
    EXPECT_EQ(get_extension("/home/user.v1/data"), "/home/user.v1/data");
}

TEST(FormatExtensionTest, HandlesDotBeforeSlash) {
    EXPECT_EQ(get_extension("dir.with.dot/file"), "dir.with.dot/file");
    EXPECT_EQ(get_extension(".hidden.dir/file.txt"), ".txt");
}

TEST(FormatExtensionTest, HandlesEdgeCases) {
    EXPECT_EQ(get_extension(""), "");
    EXPECT_EQ(get_extension("/path/."), ".");
    EXPECT_EQ(get_extension("just.a.dot."), ".");
}
