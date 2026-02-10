#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "config.hpp"
#include "dependency.hpp"
#include <string>
#include <vector>

class Compiler {
public:
    Compiler(const BuildConfig& config);
    
    // 执行完整的构建流程
    bool build();
    
    // 清理构建文件
    bool clean();
    
    // 重新构建（清理后构建）
    bool rebuild();
    
private:
    BuildConfig config;
    DependencyChecker depChecker;
    std::vector<std::string> objectFiles;
    
    // 创建构建目录
    bool createBuildDir();
    
    // 编译单个源文件
    bool compileSource(const std::string& sourceFile, const std::string& objectFile);
    
    // 链接所有目标文件
    bool linkObjects();
    
    // 构建编译命令
    std::string buildCompileCommand(const std::string& sourceFile, const std::string& objectFile);
    
    // 构建链接命令
    std::string buildLinkCommand();
    
    // 执行系统命令
    bool executeCommand(const std::string& command);
    
    // 获取目标文件路径
    std::string getObjectFilePath(const std::string& sourceFile);
    
    // 获取输出文件路径
    std::string getOutputFilePath();
};

#endif // COMPILER_HPP
