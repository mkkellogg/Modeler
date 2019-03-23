#pragma once

#include <string>

class FileUtil {
public:
    FileUtil();
    static std::string removePrefix(const std::string& str, const std::string& prefix);
    static std::string extractFileNameFromPath(const std::string& path, bool removeFileTypeSuffix);
};

