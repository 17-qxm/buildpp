#ifndef DEPENDENCY_HPP
#define DEPENDENCY_HPP

#include <string>
#include <map>
#include <sys/stat.h>

class DependencyChecker {
public:
    DependencyChecker();
    
    // 检查源文件是否需要重新编译
    bool needsRecompile(const std::string& sourceFile, const std::string& objectFile);
    
    // 获取文件的最后修改时间
    time_t getFileModTime(const std::string& filename);
    
    // 检查文件是否存在
    bool fileExists(const std::string& filename);
    
private:
    std::map<std::string, time_t> fileTimeCache;
};

#endif // DEPENDENCY_HPP
