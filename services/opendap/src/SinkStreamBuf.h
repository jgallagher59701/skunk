//
// Created by James Gallagher on 6/27/25.
//

#ifndef SINKSTREAMBUF_H
#define SINKSTREAMBUF_H

// This file includes the chunked_ostream, chunked_outbuf, and chunked_stream.h logic
// extended with an adapter for cpp-httplib streaming integration.

#include <httplib.h>
#include <streambuf>
#include <ostream>
#include <memory>
#include <functional>

#include "libdap/chunked_ostream.h"

namespace libdap {

// Streambuf that writes to a cpp-httplib DataSink
class SinkStreamBuf : public std::streambuf {
    httplib::DataSink &sink;
    std::vector<char> buffer;

public:
    SinkStreamBuf(httplib::DataSink &sink_, size_t buf_size = 8192)
        : sink(sink_), buffer(buf_size) {
        setp(buffer.data(), buffer.data() + buffer.size() - 1);
    }

protected:
    int_type overflow(int_type ch) override {
        if (ch != traits_type::eof()) {
            *pptr() = static_cast<char>(ch);
            pbump(1);
        }
        return flush_buffer() ? ch : traits_type::eof();
    }

    int sync() override {
        return flush_buffer() ? 0 : -1;
    }

private:
    bool flush_buffer() {
        std::ptrdiff_t size = pptr() - pbase();
        if (size > 0) {
            if (!sink.write(pbase(), static_cast<size_t>(size))) return false;
            pbump(-size);
        }
        return true;
    }
};

// Function to stream chunked_ostream into httplib::Response
inline void stream_chunked_response(httplib::Response &res,
                                    const std::string &content_type,
                                    std::function<void(std::ostream &)> stream_logic) {
    res.set_content_provider(
        content_type.c_str(),
        -1,  // chunked transfer encoding
        [stream_logic](size_t /*offset*/, size_t /*length*/, httplib::DataSink &sink) {
            SinkStreamBuf sbuf(sink);
            std::ostream os(&sbuf);
            chunked_ostream cos(os, CHUNK_SIZE);
            stream_logic(cos);  // Call user-supplied logic
            cos.write_end_chunk();
            return true;
        },
        []() {
            // Optional cleanup
        });
}

} // namespace libdap

#endif //SINKSTREAMBUF_H
