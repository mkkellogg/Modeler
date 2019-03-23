#include "FileUtil.h"

FileUtil::FileUtil() {

}

std::string FileUtil::removePrefix(const std::string& str, const std::string& prefix) {
    std::string strPrefix = str.substr(0, prefix.size()) ;
    if (strPrefix == prefix) {
        return str.substr(prefix.size());
    }
    return std::string(str);
}

std::string FileUtil::extractFileNameFromPath(const std::string& path, bool removeFileTypeSuffix) {
    int fileNameStart = 0;
    int fileNameEnd = path.size() - 1;
    int testIndex = path.size() - 1;
    while (testIndex >= 0) {
        auto curChar = path.at(testIndex);
        if (curChar == '.' && removeFileTypeSuffix) fileNameEnd = testIndex;
        else if(curChar == '/' || curChar == '\\') {
            fileNameStart = testIndex + 1;
            break;
        }
        testIndex--;
    }

    return path.substr(fileNameStart, fileNameEnd - fileNameStart);
}
