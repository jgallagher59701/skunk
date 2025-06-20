//
// Created by James Gallagher on 6/5/25.
//

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <ctime>
#include <stdexcept>
#include <memory>

#include "handler.h"
#include "DataAccessNetCDF.h"

using namespace std;

namespace hyrax {

///@{
/// Those should be configuration params in a real server. jhrg 6/19/25
const string data_root = "/Users/jimg/src/opendap/skunk/services/opendap/src/"; // FIXME jhrg 6/16/25
constexpr size_t BUF_SIZE = 65536;
///@}

/**
 * @brief Given a filename, return it's extension
 *
 * This code looks at the path and tries to find what a person would normally
 * call 'the file extension.' Here are examples from the unit tests:
 * "file.txt" --> ".txt"
 * "/home/user/archive.tar.gz" --> ".gz"
 * ".hidden.dir/file.txt" --> ".txt"
 * Here's some examples of something that does not have an extension:
 * "README" --> "README"
 * "/home/user/.config/file" --> "/home/user/.config/file"
 *
 * @param filename The filename
 * @return The extension or the original filename if nothing matching the
 * notion of an extension was found.
 */
string get_extension(const string &filename)
{
    // Find the last path separator
    const size_t last_slash_pos = filename.find_last_of('/');

    // Isolate the last path component (the file name)
    const string basename = (last_slash_pos != string::npos)
                            ? filename.substr(last_slash_pos + 1)
                            : filename;

    // Look for the last dot in the basename
    const size_t last_dot_pos = basename.rfind('.');
    if (last_dot_pos == string::npos) {
        // No extension in the filename part → return original input
        return filename;
    }

    // Otherwise, return the extension
    return basename.substr(last_dot_pos);
}

/**
 * @brief Assume formats are determined by extension.
 * @param data_path
 * @return
 */
data_format find_format(const string &data_path)
{
    auto format = get_extension(data_path);
    if (format == ".nc")
        return nc;
    return unknown_format;
}

/**
 * @brief Build response headers for a DAP4 DMR response
 * @note nginx adds: Server, Date, Connection, and Content-Length
 * (modifies Content-Length, I think, because cpp-httplib sets it.)
 * @param res The response object
 * @param date The date to use for the Last Modified Time
 */
void set_dmr_response_headers(httplib::Response &res, const string &date)
{
    res.set_header("Cache-Control", "public");
    res.set_header("Last-Modified", date);
    res.set_header("X-FRAME-OPTIONS", "DENY");
    res.set_header("XDODS-Server", "dods/3.2");
    res.set_header("X-DAP", "4.0");
    res.set_header("XOPeNDAP-Server", "s-works");
    res.set_header("Content-Description", "application/vnd.opendap.dap4.dataset-metadata+xml");
    res.set_header("Content-Type", "application/vnd.opendap.dap4.dataset-metadata+xml");
}

/**
 * @brief Get the Last Modified Time (LMT) for a file
 * @param filepath Pathname of the file
 * @return A unix time value for the file's LMT
 */
time_t get_last_modification_time(const string &filepath)
{
    struct stat file_info{};
    if (stat(filepath.c_str(), &file_info) != 0) {
        throw runtime_error("Unable to retrieve file info: " + filepath);
    }
    return file_info.st_mtime;
}

/**
 * @brief Convert a unix time value to a string for use with HTTP
 * @note IIRC, this is formally a RFC 1023 conformant string
 * @param t The unix time value
 * @return A HTTP header-ready time string
 */
string format_http_date(time_t t)
{
    char buf[100];
    tm tm{};
    gmtime_r(&t, &tm);  // Thread-safe UTC conversion
    if (strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm) == 0) {
        throw runtime_error("strftime failed");
    }
    return {buf};
}

/**
 * @brief Return a DMR response for the given data_path
 *
 * @note The data root, used to convert the data_path to a true file pathname
 * is a hack
 * @note This only works for 'fnoc1.nc' and will otherwise just return 'Moof!'
 *
 * @param data_path The path to the data. A 'service-relative' path.
 * @param req The cpp-httplib Request object
 * @param res The cpp-httplib Response object
 */
void handle_dmr_request(const string &data_path, const httplib::Request &req, httplib::Response &res)
{
    // extract info from headers, etc.
    const auto ce = req.has_param("dap4_ce") ? req.get_param_value("dap4_ce") : "";
    const auto function = req.has_param("dap4_function") ? req.get_param_value("dap4_function") : "";

    const auto format = find_format(data_path);
    if (format == nc) {
        if (data_path.find("fnoc1.nc") != string::npos) {
            const auto full_data_path = data_root + data_path + ".dmr";
            const auto lmt_date = format_http_date(get_last_modification_time(full_data_path));
            set_dmr_response_headers(res, lmt_date);
#if 0
            res.set_file_content(full_data_path, "application/vnd.opendap.dap4.dataset-metadata+xml");
#else
            // Cannot use a unique_ptr here; those are not copyable. jhrg 6/19/25
            auto file = std::make_shared<std::ifstream>(full_data_path, std::ios::binary | std::ios::ate);
            if (!file->is_open()) {
                res.status = 404;
                res.set_content("DMR file not found", "text/plain");
                return;
            }

            auto size = file->tellg();
            file->seekg(0);

            res.set_content_provider(
                    static_cast<size_t>(size),
                    "application/vnd.opendap.dap4.dataset-metadata+xml",
                    [file = std::move(file)](size_t offset, size_t length, httplib::DataSink &sink) mutable {
                //char buffer[65535];
                vector<char> buffer(BUF_SIZE);
                file->seekg(static_cast<streamoff>(offset));
                file->read(buffer.data(), static_cast<streamsize>(std::min(BUF_SIZE, length)));
                sink.write(buffer.data(), file->gcount());
                return true; // continue sending
            });
#endif

#if 0
            DataAccessNetCDF format_handler;
            res.set_content(format_handler.get_dmr_file(full_data_path),
                            "application/vnd.opendap.dap4.dataset-metadata+xml");
#endif
            return;
        }

        res.set_content("Moof!", "text/plain");
        return;
    }

    res.set_content("Error: only netCDF files can be served.", "text/plain");
}

}