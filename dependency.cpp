#include "dependency.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

DependencyChecker::DependencyChecker() {
}

bool DependencyChecker::fileExists(const std::string& filename) {
#ifdef _WIN32
    struct _stat buffer;
    return (_stat(filename.c_str(), &buffer) == 0);
#else
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
#endif
}

time_t DependencyChecker::getFileModTime(const std::string& filename) {
    // 检查缓存
    auto it = fileTimeCache.find(filename);
    if (it != fileTimeCache.end()) {
        return it->second;
    }
    
#ifdef _WIN32
    struct _stat fileInfo;
    if (_stat(filename.c_str(), &fileInfo) != 0) {
        return 0;
    }
#else
    struct stat fileInfo;
    if (stat(filename.c_str(), &fileInfo) != 0) {
        return 0;
    }
#endif
    
    time_t modTime = fileInfo.st_mtime;
    fileTimeCache[filename] = modTime;
    return modTime;
}

bool DependencyChecker::needsRecompile(const std::string& sourceFile, const std::string& objectFile) {
    // 如果目标文件不存在，需要编译
    if (!fileExists(objectFile)) {
        return true;
    }
    
    // 如果源文件不存在，报错
    if (!fileExists(sourceFile)) {
        std::cerr << "Error: Source file does not exist: " << sourceFile << std::endl;
        return false;
    }
    
    // 比较修改时间
    time_t sourceTime = getFileModTime(sourceFile);
    time_t objectTime = getFileModTime(objectFile);
    
    // 如果源文件比目标文件新，需要重新编译
    return sourceTime > objectTime;
}
