#include "compiler.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir(dir, mode) _mkdir(dir)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

Compiler::Compiler(const BuildConfig& config) : config(config) {
}

bool Compiler::createBuildDir() {
    if (!depChecker.fileExists(config.build_dir)) {
        std::cout << "Creating build directory: " << config.build_dir << std::endl;
#ifdef _WIN32
        if (_mkdir(config.build_dir.c_str()) != 0) {
#else
        if (mkdir(config.build_dir.c_str(), 0755) != 0) {
#endif
            std::cerr << "Error: Failed to create build directory" << std::endl;
            return false;
        }
    }
    return true;
}

std::string Compiler::getObjectFilePath(const std::string& sourceFile) {
    // 从源文件路径提取文件名
    size_t lastSlash = sourceFile.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? 
                          sourceFile.substr(lastSlash + 1) : sourceFile;
    
    // 替换扩展名为 .o
    size_t lastDot = filename.find_last_of(".");
    if (lastDot != std::string::npos) {
        filename = filename.substr(0, lastDot);
    }
    
    return config.build_dir + "/" + filename + ".o";
}

std::string Compiler::getOutputFilePath() {
    std::string outputName = config.output_name.empty() ? 
                            config.project_name : config.output_name;
    
#ifdef _WIN32
    if (config.output_type == "executable") {
        outputName += ".exe";
    } else {
        outputName += ".dll";
    }
#else
    if (config.output_type == "library") {
        outputName = "lib" + outputName + ".so";
    }
#endif
    
    return config.build_dir + "/" + outputName;
}

std::string Compiler::buildCompileCommand(const std::string& sourceFile, 
                                         const std::string& objectFile) {
    std::stringstream cmd;
    cmd << config.compiler << " ";
    
    // C++ 标准
    cmd << "-std=" << config.cpp_standard << " ";
    
    // 优化级别
    cmd << "-" << config.optimization << " ";
    
    // 调试信息
    if (config.debug) {
        cmd << "-g ";
    }
    
    // 包含目录
    for (const auto& includeDir : config.include_dirs) {
        cmd << "-I" << includeDir << " ";
    }
    
    // 额外的编译标志
    for (const auto& flag : config.compile_flags) {
        cmd << flag << " ";
    }
    
    // 编译为目标文件
    cmd << "-c " << sourceFile << " -o " << objectFile;
    
    return cmd.str();
}

std::string Compiler::buildLinkCommand() {
    std::stringstream cmd;
    cmd << config.compiler << " ";
    
    // 所有目标文件
    for (const auto& objFile : objectFiles) {
        cmd << objFile << " ";
    }
    
    // 库目录
    for (const auto& libDir : config.library_dirs) {
        cmd << "-L" << libDir << " ";
    }
    
    // 库文件
    for (const auto& lib : config.libraries) {
        cmd << "-l" << lib << " ";
    }
    
    // 额外的链接标志
    for (const auto& flag : config.link_flags) {
        cmd << flag << " ";
    }
    
    // 输出文件
    cmd << "-o " << getOutputFilePath();
    
    // 如果是共享库
    if (config.output_type == "library") {
        cmd << " -shared";
    }
    
    return cmd.str();
}

bool Compiler::executeCommand(const std::string& command) {
    std::cout << "Executing: " << command << std::endl;
    int result = system(command.c_str());
    return result == 0;
}

bool Compiler::compileSource(const std::string& sourceFile, 
                            const std::string& objectFile) {
    // 检查是否需要重新编译
    if (!depChecker.needsRecompile(sourceFile, objectFile)) {
        std::cout << "Skipping " << sourceFile << " (up to date)" << std::endl;
        return true;
    }
    
    std::cout << "Compiling " << sourceFile << "..." << std::endl;
    std::string command = buildCompileCommand(sourceFile, objectFile);
    
    if (!executeCommand(command)) {
        std::cerr << "Error: Failed to compile " << sourceFile << std::endl;
        return false;
    }
    
    return true;
}

bool Compiler::linkObjects() {
    std::cout << "\nLinking..." << std::endl;
    std::string command = buildLinkCommand();
    
    if (!executeCommand(command)) {
        std::cerr << "Error: Failed to link" << std::endl;
        return false;
    }
    
    return true;
}

bool Compiler::build() {
    std::cout << "\n=== Starting Build ===" << std::endl;
    std::cout << "Project: " << config.project_name << std::endl;
    std::cout << "Output: " << getOutputFilePath() << "\n" << std::endl;
    
    // 创建构建目录
    if (!createBuildDir()) {
        return false;
    }
    
    // 编译所有源文件
    objectFiles.clear();
    bool allCompiled = true;
    
    for (const auto& sourceFile : config.source_files) {
        std::string objectFile = getObjectFilePath(sourceFile);
        objectFiles.push_back(objectFile);
        
        if (!compileSource(sourceFile, objectFile)) {
            allCompiled = false;
            break;
        }
    }
    
    if (!allCompiled) {
        std::cerr << "\nBuild failed during compilation" << std::endl;
        return false;
    }
    
    // 链接
    if (!linkObjects()) {
        std::cerr << "\nBuild failed during linking" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Build Successful ===" << std::endl;
    std::cout << "Output: " << getOutputFilePath() << std::endl;
    return true;
}

bool Compiler::clean() {
    std::cout << "Cleaning build directory..." << std::endl;
    
#ifdef _WIN32
    std::string command = "rmdir /s /q " + config.build_dir;
#else
    std::string command = "rm -rf " + config.build_dir;
#endif
    
    if (depChecker.fileExists(config.build_dir)) {
        system(command.c_str());
        std::cout << "Clean complete" << std::endl;
    } else {
        std::cout << "Build directory does not exist" << std::endl;
    }
    
    return true;
}

bool Compiler::rebuild() {
    std::cout << "=== Rebuilding ===" << std::endl;
    clean();
    return build();
}
