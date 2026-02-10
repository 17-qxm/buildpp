#include "config.hpp"
#include "compiler.hpp"
#include <iostream>
#include <string>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [config_file]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  init               Initialize a new build.json" << std::endl;
    std::cout << "  build              Build the project (default)" << std::endl;
    std::cout << "  clean              Clean build artifacts" << std::endl;
    std::cout << "  rebuild            Clean and rebuild" << std::endl;
    std::cout << "  -h, --help         Show this help message" << std::endl;
    std::cout << "  -v, --verbose      Show configuration details" << std::endl;
    std::cout << "\nConfig file: build.json (default)" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << programName << " init               # Initialize new project" << std::endl;
    std::cout << "  " << programName << "                    # Build using build.json" << std::endl;
    std::cout << "  " << programName << " clean              # Clean build directory" << std::endl;
    std::cout << "  " << programName << " rebuild            # Clean and rebuild" << std::endl;
    std::cout << "  " << programName << " myconfig.json      # Build using custom config" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string configFile = "build.json";
    std::string command = "build";
    bool verbose = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "build" || arg == "clean" || arg == "rebuild" || arg == "init") {
            command = arg;
        } else if (arg.find(".json") != std::string::npos) {
            configFile = arg;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    std::cout << "=== C++ Build Helper ===" << std::endl;
    std::cout << "Config file: " << configFile << std::endl;
    std::cout << "Command: " << command << "\n" << std::endl;
    
    // 处理 init 命令
    if (command == "init") {
        bool configSuccess = ConfigParser::generateDefaultConfig(configFile);
        bool guideSuccess = ConfigParser::generateGuideDocument("BUILD_GUIDE.md");
        
        if (configSuccess && guideSuccess) {
            std::cout << "Successfully created:\n";
            std::cout << "  - " << configFile << " (configuration file)\n";
            std::cout << "  - BUILD_GUIDE.md (documentation)\n\n";
            std::cout << "Please read BUILD_GUIDE.md for detailed configuration options.\n";
            std::cout << "Edit " << configFile << " to configure your project.\n";
            return 0;
        } else {
            std::cerr << "Failed to initialize project files\n";
            return 1;
        }
    }
    
    // 加载配置
    ConfigParser parser;
    if (!parser.loadFromFile(configFile)) {
        std::cerr << "Failed to load configuration file: " << configFile << std::endl;
        return 1;
    }
    
    // 显示配置（如果启用verbose）
    if (verbose) {
        parser.printConfig();
        std::cout << std::endl;
    }
    
    // 创建编译器并执行命令
    Compiler compiler(parser.getConfig());
    bool success = false;
    
    if (command == "build") {
        success = compiler.build();
    } else if (command == "clean") {
        success = compiler.clean();
    } else if (command == "rebuild") {
        success = compiler.rebuild();
    }
    
    if (success) {
        std::cout << "\nDone!" << std::endl;
        return 0;
    } else {
        std::cerr << "\nFailed!" << std::endl;
        return 1;
    }
}
