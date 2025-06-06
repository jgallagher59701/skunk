//
// Created by James Gallagher on 6/5/25.
//

#ifndef SKUNK_HANDLER_H
#define SKUNK_HANDLER_H

#include <string>

#include "httplib.h"

std::string handle_request(const httplib::Request& req);

#endif //SKUNK_HANDLER_H
