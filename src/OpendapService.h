//
// Created by James Gallagher on 5/26/25.
//

#ifndef SKUNK_OPENDAPSERVICE_H
#define SKUNK_OPENDAPSERVICE_H

/**
 * @brief The OPeNDAP service.
 *
 * This extends ServiceBase to provide the OPeNDAP service.
 * It uses dependency injection to use data-type specific
 * operations.
 */
class OpendapService : public ServiceBase {
    ReadData &read_data;
public:
    OpendapService() = delete;
    explicit OpendapService(ReadData &read_data);

    OpendapService(const OpendapService &) = delete;
    OpendapService &operator=(const OpendapService &) = delete;
    OpendapService(const OpendapService &&) = delete;
    OpendapService &operator=(const OpendapService &&) = delete;

    ~OpendapService() = default;
};

};


#endif //SKUNK_OPENDAPSERVICE_H
