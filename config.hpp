#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct BuildConfig {
    std::string project_name;
    std::string output_name;
    std::string output_type; // "executable" or "library"
    std::string cpp_standard; // "c++11", "c++14", "c++17", "c++20", etc.
    std::string optimization; // "O0", "O1", "O2", "O3", "Os"
    bool debug;
    
    std::vector<std::string> source_files;
    std::vector<std::string> include_dirs;
    std::vector<std::string> library_dirs;
    std::vector<std::string> libraries;
    std::vector<std::string> compile_flags;
    std::vector<std::string> link_flags;
    
    std::string build_dir;
    std::string compiler; // "g++" or "gcc"
};

class ConfigParser {
public:
    ConfigParser();
    bool loadFromFile(const std::string& filename);
    const BuildConfig& getConfig() const { return config; }
    void printConfig() const;
    
    // 生成默认配置文件和文档
    static bool generateDefaultConfig(const std::string& filename);
    static bool generateGuideDocument(const std::string& filename);
    
private:
    BuildConfig config;
    bool parseJson(const std::string& content);
    std::string trim(const std::string& str);
    std::string extractString(const std::string& json, const std::string& key);
    std::vector<std::string> extractArray(const std::string& json, const std::string& key);
    bool extractBool(const std::string& json, const std::string& key, bool defaultValue = false);
    
    // 文件夹扫描相关方法
    std::vector<std::string> expandSourceFiles(const std::vector<std::string>& entries);
    bool isDirectory(const std::string& path);
    std::vector<std::string> listCppFiles(const std::string& directory);
    bool isCppFile(const std::string& filename);
};

#endif // CONFIG_HPP
