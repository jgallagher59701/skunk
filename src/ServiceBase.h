//
// Created by James Gallagher on 5/26/25.
//

#ifndef SKUNK_SERVICEBASE_H
#define SKUNK_SERVICEBASE_H

namespace hyrax {

/**
 * @brief Base class for all services.
 *
 */
class ServiceBase {
    virtual send_headers();
    virtual send_string_response();
    virtual stream_data_response();
};

} // hyrax

#endif //SKUNK_SERVICEBASE_H
