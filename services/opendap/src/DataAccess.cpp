//
// Created by James Gallagher on 5/27/25.
//

#include <iostream>
#include <fstream>
#include <string>
#include "DataAccess.h"

using namespace std;

string DataAccess::get_dmr_file(const std::string &path)
{
    ifstream is;
    is.open(path, ios::binary);

    if (!is)
        return {"Could not read DMR/XML file: " + path};

    // get length of the file
    is.seekg(0, ios::end);
    auto length = is.tellg();

    // back to start
    is.seekg(0, ios::beg);

    // allocate memory:
    vector<char> buffer(length);

    // read data as a block:
    is.read(buffer.data(), length);
    is.close();

    buffer.push_back('\0');

    return {buffer.data()};
}