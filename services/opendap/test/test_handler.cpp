
// Unit tests for hyrax handler functions

#include <ctime>
#include <sys/stat.h>

#include <gtest/gtest.h>

#include "httplib.h"
#include "handler.h"

using namespace hyrax;

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
// Test find_format()
TEST(FindFormatTest, HandlesKnownFormat) {
    EXPECT_EQ(find_format("data.nc"), nc);
}

TEST(FindFormatTest, HandlesUnknownFormat) {
    EXPECT_EQ(find_format("data.txt"), unknown_format);
    EXPECT_EQ(find_format("data"), unknown_format);
    EXPECT_EQ(find_format("data.nc.extra"), unknown_format);
}

// Test format_http_date()
TEST(FormatHttpDateTest, FormatsCorrectly) {
    std::time_t known_time = 0; // Example fixed time
    std::string formatted = format_http_date(known_time);
    EXPECT_EQ(formatted, "Thu, 01 Jan 1970 00:00:00 GMT");
}

TEST(FormatHttpDateTest, FormatsCorrectlyEndTimes) {
    std::time_t known_time = 0xFFFF'FFFF; // Example fixed time
    std::string formatted = format_http_date(known_time);
    EXPECT_EQ(formatted, "Sun, 07 Feb 2106 06:28:15 GMT");
}

TEST(FormatHttpDateTest, ThrowsOnInvalidInput) {
    // Not directly testable easily, since strftime rarely fails unless the buffer is tiny
    // Hence, we only do a smoke test here.
    EXPECT_NO_THROW(format_http_date(0));
}

// Test get_last_modification_time()
TEST(GetLastModificationTimeTest, HandlesExistingFile) {
    const std::string filename = "."; // this file always exists in the CWD
    EXPECT_NO_THROW({
                        auto mod_time = get_last_modification_time(filename);
                        EXPECT_GT(mod_time, 0);
                    });
}

TEST(GetLastModificationTimeTest, ThrowsOnMissingFile) {
    const std::string filename = "nonexistent.file";
    EXPECT_THROW(get_last_modification_time(filename), std::runtime_error);
}

// Test set_dmr_response_headers()
TEST(SetDmrResponseHeadersTest, SetsAllHeaders) {
    httplib::Response res;
    std::string test_date = "Sun, 16 Jun 2024 20:28:35 GMT";

    set_dmr_response_headers(res, test_date);

    EXPECT_EQ(res.get_header_value("Cache-Control"), "public");
    EXPECT_EQ(res.get_header_value("Last-Modified"), test_date);
    EXPECT_EQ(res.get_header_value("X-FRAME-OPTIONS"), "DENY");
    EXPECT_EQ(res.get_header_value("XDODS-Server"), "dods/3.2");
    EXPECT_EQ(res.get_header_value("X-DAP"), "4.0");
    EXPECT_EQ(res.get_header_value("XOPeNDAP-Server"), "s-works");
    EXPECT_EQ(res.get_header_value("Content-Description"), "application/vnd.opendap.dap4.dataset-metadata+xml");
    EXPECT_EQ(res.get_header_value("Content-Type"), "application/vnd.opendap.dap4.dataset-metadata+xml");
}

// handle_dmr_request() is complex and involves file operations and HTTP streaming.
// Typically, this would require integration tests rather than pure unit tests.
// However, we can at least test the behavior when file format is unknown.

TEST(HandleDmrRequestTest, HandlesUnknownFormat) {
    httplib::Request req;
    httplib::Response res;

    handle_dmr_request("unknown_format.xyz", req, res);

    EXPECT_EQ(res.status, 200);
    EXPECT_EQ(res.body, "Error: only netCDF files can be served.");
    EXPECT_EQ(res.get_header_value("Content-Type"), "text/plain");
}

TEST(HandleDmrRequestTest, HandlesKnownNcFormatButMissingFile) {
    httplib::Request req;
    httplib::Response res;

    // Assuming fnoc1.nc does NOT exist in the test directory
    handle_dmr_request("fnoc1.nc", req, res);

    EXPECT_EQ(res.status, 404);
    EXPECT_EQ(res.body, "DMR file not found");
    EXPECT_EQ(res.get_header_value("Content-Type"), "text/plain");
}

// Additional extensive testing of handle_dmr_request would typically involve mocks or fixtures
// and thus would be integration-level testing.
