#pragma once

#include <string>

class Exception {
public:
    Exception(const std::string& msg);
    const std::string msg;
};
