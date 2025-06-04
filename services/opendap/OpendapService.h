//
// Created by James Gallagher on 5/26/25.
//

#ifndef SKUNK_OPENDAPSERVICE_H
#define SKUNK_OPENDAPSERVICE_H

#include "ServiceBase.h"

/**
 * @brief The OPeNDAP service.
 *
 * This extends ServiceBase to provide the OPeNDAP service.
 * It uses dependency injection to use data-type specific
 * operations.
 */
class OpendapService : public ServiceBase {
    /// This is an instance of the DataAccess class provides the 'how' for data access.
    DataAccess &data_access;

public:
    OpendapService() = delete;
    explicit OpendapService(DataAccess &data_access);

    OpendapService(const OpendapService &) = delete;
    OpendapService &operator=(const OpendapService &) = delete;
    // @TODO Fix this - maybe we want the move stuff? jhrg 5/2/25
    OpendapService(const OpendapService &&) = delete;
    OpendapService &operator=(const OpendapService &&) = delete;

    ~OpendapService() override = default;
};

};


#endif //SKUNK_OPENDAPSERVICE_H
