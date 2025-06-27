//
// Created by James Gallagher on 6/27/25.
//

#include "chunked_stream_server"  // or wherever the integrated code resides

svr.Get("/dap4", [](const httplib::Request &req, httplib::Response &res) {
    libdap::stream_chunked_response(res, "application/octet-stream", [](std::ostream &out) {
        // Use your existing serialization logic
        libdap::chunked_ostream cos(out, CHUNK_SIZE);
        // ... write to cos like normal ...
        cos << "This is a streamed chunk!\n";
        cos.flush();  // Optional: flush current buffer into a chunk
    });
});
