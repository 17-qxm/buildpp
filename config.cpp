#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <sys/stat.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

ConfigParser::ConfigParser() {
    // 设置默认值
    config.cpp_standard = "c++17";
    config.optimization = "O2";
    config.debug = false;
    config.build_dir = "build";
    config.compiler = "g++";
    config.output_type = "executable";
}

bool ConfigParser::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << filename << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    return parseJson(content);
}

std::string ConfigParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\"");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\",\"");
    return str.substr(first, last - first + 1);
}

std::string ConfigParser::extractString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    
    size_t start = json.find("\"", pos);
    if (start == std::string::npos) return "";
    start++;
    
    size_t end = json.find("\"", start);
    if (end == std::string::npos) return "";
    
    return json.substr(start, end - start);
}

std::vector<std::string> ConfigParser::extractArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return result;
    
    pos = json.find("[", pos);
    if (pos == std::string::npos) return result;
    
    size_t end = json.find("]", pos);
    if (end == std::string::npos) return result;
    
    std::string arrayContent = json.substr(pos + 1, end - pos - 1);
    
    size_t start = 0;
    while (true) {
        size_t quoteStart = arrayContent.find("\"", start);
        if (quoteStart == std::string::npos) break;
        
        size_t quoteEnd = arrayContent.find("\"", quoteStart + 1);
        if (quoteEnd == std::string::npos) break;
        
        std::string item = arrayContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
        if (!item.empty()) {
            result.push_back(item);
        }
        
        start = quoteEnd + 1;
    }
    
    return result;
}

bool ConfigParser::extractBool(const std::string& json, const std::string& key, bool defaultValue) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return defaultValue;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return defaultValue;
    
    size_t truePos = json.find("true", pos);
    size_t falsePos = json.find("false", pos);
    size_t nextKey = json.find("\"", pos + 1);
    
    if (truePos != std::string::npos && (nextKey == std::string::npos || truePos < nextKey)) {
        return true;
    }
    if (falsePos != std::string::npos && (nextKey == std::string::npos || falsePos < nextKey)) {
        return false;
    }
    
    return defaultValue;
}

bool ConfigParser::parseJson(const std::string& content) {
    // 简单的JSON解析（针对我们的配置格式）
    config.project_name = extractString(content, "project_name");
    config.output_name = extractString(content, "output_name");
    config.output_type = extractString(content, "output_type");
    config.cpp_standard = extractString(content, "cpp_standard");
    config.optimization = extractString(content, "optimization");
    config.debug = extractBool(content, "debug", false);
    config.build_dir = extractString(content, "build_dir");
    config.compiler = extractString(content, "compiler");
    
    // 如果某些字段为空，使用默认值
    if (config.cpp_standard.empty()) config.cpp_standard = "c++17";
    if (config.optimization.empty()) config.optimization = "O2";
    if (config.build_dir.empty()) config.build_dir = "build";
    if (config.compiler.empty()) config.compiler = "g++";
    if (config.output_type.empty()) config.output_type = "executable";
    
    std::vector<std::string> rawSourceFiles = extractArray(content, "source_files");
    config.source_files = expandSourceFiles(rawSourceFiles);
    config.include_dirs = extractArray(content, "include_dirs");
    config.library_dirs = extractArray(content, "library_dirs");
    config.libraries = extractArray(content, "libraries");
    config.compile_flags = extractArray(content, "compile_flags");
    config.link_flags = extractArray(content, "link_flags");
    
    if (config.project_name.empty()) {
        std::cerr << "Error: project_name is required in config file" << std::endl;
        return false;
    }
    
    if (config.source_files.empty()) {
        std::cerr << "Error: source_files is required in config file" << std::endl;
        return false;
    }
    
    return true;
}

void ConfigParser::printConfig() const {
    std::cout << "=== Build Configuration ===" << std::endl;
    std::cout << "Project: " << config.project_name << std::endl;
    std::cout << "Output: " << config.output_name << std::endl;
    std::cout << "Type: " << config.output_type << std::endl;
    std::cout << "Compiler: " << config.compiler << std::endl;
    std::cout << "C++ Standard: " << config.cpp_standard << std::endl;
    std::cout << "Optimization: " << config.optimization << std::endl;
    std::cout << "Debug: " << (config.debug ? "Yes" : "No") << std::endl;
    std::cout << "Build Dir: " << config.build_dir << std::endl;
    
    std::cout << "\nSource Files (" << config.source_files.size() << "):" << std::endl;
    for (const auto& file : config.source_files) {
        std::cout << "  - " << file << std::endl;
    }
    
    if (!config.include_dirs.empty()) {
        std::cout << "\nInclude Directories:" << std::endl;
        for (const auto& dir : config.include_dirs) {
            std::cout << "  - " << dir << std::endl;
        }
    }
    
    if (!config.library_dirs.empty()) {
        std::cout << "\nLibrary Directories:" << std::endl;
        for (const auto& dir : config.library_dirs) {
            std::cout << "  - " << dir << std::endl;
        }
    }
    
    if (!config.libraries.empty()) {
        std::cout << "\nLibraries:" << std::endl;
        for (const auto& lib : config.libraries) {
            std::cout << "  - " << lib << std::endl;
        }
    }
    
    std::cout << "===========================" << std::endl;
}

// 生成默认配置文件
bool ConfigParser::generateDefaultConfig(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create config file: " << filename << std::endl;
        return false;
    }
    
    file << "{\n";
    file << "  \"project_name\": \"my_project\",\n";
    file << "  \"output_name\": \"\",\n";
    file << "  \"output_type\": \"executable\",\n";
    file << "  \"compiler\": \"g++\",\n";
    file << "  \"cpp_standard\": \"c++17\",\n";
    file << "  \"optimization\": \"O2\",\n";
    file << "  \"debug\": false,\n";
    file << "  \"build_dir\": \"build\",\n";
    file << "  \"source_files\": [\n";
    file << "    \"main.cpp\"\n";
    file << "  ],\n";
    file << "  \"include_dirs\": [],\n";
    file << "  \"library_dirs\": [],\n";
    file << "  \"libraries\": [],\n";
    file << "  \"compile_flags\": [\"-Wall\", \"-Wextra\"],\n";
    file << "  \"link_flags\": []\n";
    file << "}\n";
    
    file.close();
    return true;
}

// 生成帮助文档
bool ConfigParser::generateGuideDocument(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create guide file: " << filename << std::endl;
        return false;
    }
    
    file << "# Build Configuration Guide\n\n";
    file << "This guide explains all configuration options for the C++ Build Helper.\n\n";
    
    file << "## Quick Start\n\n";
    file << "1. Initialize a new project:\n";
    file << "   ```bash\n";
    file << "   ./buildhelper init\n";
    file << "   ```\n\n";
    file << "2. Edit `build.json` with your project settings\n\n";
    file << "3. Build your project:\n";
    file << "   ```bash\n";
    file << "   ./buildhelper build\n";
    file << "   ```\n\n";
    
    file << "## Configuration Fields\n\n";
    
    file << "### Required Fields\n\n";
    file << "| Field | Type | Description |\n";
    file << "|-------|------|-------------|\n";
    file << "| `project_name` | string | Project name (used as default output name) |\n";
    file << "| `source_files` | array | List of source files or directories to compile |\n\n";
    
    file << "### Optional Fields\n\n";
    file << "| Field | Type | Default | Description |\n";
    file << "|-------|------|---------|-------------|\n";
    file << "| `output_name` | string | `project_name` | Name of the output executable/library |\n";
    file << "| `output_type` | string | `\"executable\"` | Output type: `\"executable\"` or `\"library\"` |\n";
    file << "| `compiler` | string | `\"g++\"` | Compiler to use: `\"g++\"` or `\"gcc\"` |\n";
    file << "| `cpp_standard` | string | `\"c++17\"` | C++ standard version |\n";
    file << "| `optimization` | string | `\"O2\"` | Optimization level |\n";
    file << "| `debug` | boolean | `false` | Include debug symbols |\n";
    file << "| `build_dir` | string | `\"build\"` | Directory for build artifacts |\n";
    file << "| `include_dirs` | array | `[]` | Header file search paths |\n";
    file << "| `library_dirs` | array | `[]` | Library search paths |\n";
    file << "| `libraries` | array | `[]` | Libraries to link against |\n";
    file << "| `compile_flags` | array | `[]` | Additional compiler flags |\n";
    file << "| `link_flags` | array | `[]` | Additional linker flags |\n\n";
    
    file << "## Field Details\n\n";
    
    file << "### project_name\n";
    file << "**Type:** string (required)  \n";
    file << "**Description:** The name of your project. Used as the default output name if `output_name` is not specified.\n\n";
    file << "**Example:**\n";
    file << "```json\n";
    file << "\"project_name\": \"my_awesome_app\"\n";
    file << "```\n\n";
    
    file << "### output_name\n";
    file << "**Type:** string (optional)  \n";
    file << "**Default:** Same as `project_name`  \n";
    file << "**Description:** Name of the final executable or library file. File extension is added automatically.\n\n";
    file << "**Example:**\n";
    file << "```json\n";
    file << "\"output_name\": \"myapp\"\n";
    file << "```\n\n";
    
    file << "### output_type\n";
    file << "**Type:** string (optional)  \n";
    file << "**Default:** `\"executable\"`  \n";
    file << "**Options:** `\"executable\"` or `\"library\"`  \n";
    file << "**Description:** Type of output to generate.\n\n";
    
    file << "### compiler\n";
    file << "**Type:** string (optional)  \n";
    file << "**Default:** `\"g++\"`  \n";
    file << "**Options:** `\"g++\"` or `\"gcc\"`  \n";
    file << "**Description:** Compiler to use for building.\n\n";
    
    file << "### cpp_standard\n";
    file << "**Type:** string (optional)  \n";
    file << "**Default:** `\"c++17\"`  \n";
    file << "**Options:** `\"c++11\"`, `\"c++14\"`, `\"c++17\"`, `\"c++20\"`, `\"c++23\"`, etc.  \n";
    file << "**Description:** C++ language standard to use.\n\n";
    
    file << "### optimization\n";
    file << "**Type:** string (optional)  \n";
    file << "**Default:** `\"O2\"`  \n";
    file << "**Options:**\n";
    file << "- `\"O0\"` - No optimization (fastest compilation)\n";
    file << "- `\"O1\"` - Basic optimization\n";
    file << "- `\"O2\"` - Moderate optimization (recommended)\n";
    file << "- `\"O3\"` - Aggressive optimization\n";
    file << "- `\"Os\"` - Optimize for size\n\n";
    
    file << "### debug\n";
    file << "**Type:** boolean (optional)  \n";
    file << "**Default:** `false`  \n";
    file << "**Description:** Include debug symbols in the output (adds `-g` flag).\n\n";
    
    file << "### build_dir\n";
    file << "**Type:** string (optional)  \n";
    file << "**Default:** `\"build\"`  \n";
    file << "**Description:** Directory where object files and output will be placed.\n\n";
    
    file << "### source_files\n";
    file << "**Type:** array (required)  \n";
    file << "**Description:** List of source files or directories to compile.\n\n";
    file << "**Features:**\n";
    file << "- **Individual files:** Specify exact file paths\n";
    file << "- **Directories:** Automatically scans for C++ files (`.cpp`, `.cc`, `.cxx`, `.c++`, `.C`)\n";
    file << "- **Non-recursive:** Directory scanning only includes files in that directory, not subdirectories\n\n";
    file << "**Example:**\n";
    file << "```json\n";
    file << "\"source_files\": [\n";
    file << "  \"main.cpp\",\n";
    file << "  \"src\",\n";
    file << "  \"utils/helper.cpp\"\n";
    file << "]\n";
    file << "```\n\n";
    
    file << "### include_dirs\n";
    file << "**Type:** array (optional)  \n";
    file << "**Default:** `[]`  \n";
    file << "**Description:** Directories to search for header files (adds `-I` flag for each).\n\n";
    file << "**Example:**\n";
    file << "```json\n";
    file << "\"include_dirs\": [\"include\", \"third_party/include\"]\n";
    file << "```\n\n";
    
    file << "### library_dirs\n";
    file << "**Type:** array (optional)  \n";
    file << "**Default:** `[]`  \n";
    file << "**Description:** Directories to search for library files (adds `-L` flag for each).\n\n";
    
    file << "### libraries\n";
    file << "**Type:** array (optional)  \n";
    file << "**Default:** `[]`  \n";
    file << "**Description:** Libraries to link against (adds `-l` flag for each). Do not include the `-l` prefix.\n\n";
    file << "**Example:**\n";
    file << "```json\n";
    file << "\"libraries\": [\"pthread\", \"curl\", \"sqlite3\"]\n";
    file << "```\n\n";
    
    file << "### compile_flags\n";
    file << "**Type:** array (optional)  \n";
    file << "**Default:** `[]`  \n";
    file << "**Description:** Additional flags to pass to the compiler during compilation phase.\n\n";
    file << "**Common flags:**\n";
    file << "- `-Wall` - Enable all warnings\n";
    file << "- `-Wextra` - Enable extra warnings\n";
    file << "- `-Werror` - Treat warnings as errors\n";
    file << "- `-pedantic` - Strict ISO C++ compliance\n";
    file << "- `-fPIC` - Position independent code (required for shared libraries)\n\n";
    
    file << "### link_flags\n";
    file << "**Type:** array (optional)  \n";
    file << "**Default:** `[]`  \n";
    file << "**Description:** Additional flags to pass to the linker during linking phase.\n\n";
    
    file << "## Examples\n\n";
    
    file << "### Example 1: Simple Executable\n\n";
    file << "```json\n";
    file << "{\n";
    file << "  \"project_name\": \"hello\",\n";
    file << "  \"source_files\": [\"main.cpp\"]\n";
    file << "}\n";
    file << "```\n\n";
    
    file << "### Example 2: Project with Multiple Files\n\n";
    file << "```json\n";
    file << "{\n";
    file << "  \"project_name\": \"calculator\",\n";
    file << "  \"source_files\": [\"main.cpp\", \"calculator.cpp\", \"parser.cpp\"],\n";
    file << "  \"cpp_standard\": \"c++17\",\n";
    file << "  \"compile_flags\": [\"-Wall\", \"-Wextra\"]\n";
    file << "}\n";
    file << "```\n\n";
    
    file << "### Example 3: Project with Directory Scanning\n\n";
    file << "```json\n";
    file << "{\n";
    file << "  \"project_name\": \"myapp\",\n";
    file << "  \"source_files\": [\"main.cpp\", \"src\", \"utils\"],\n";
    file << "  \"include_dirs\": [\"include\"],\n";
    file << "  \"cpp_standard\": \"c++20\"\n";
    file << "}\n";
    file << "```\n\n";
    
    file << "### Example 4: Project with External Libraries\n\n";
    file << "```json\n";
    file << "{\n";
    file << "  \"project_name\": \"network_app\",\n";
    file << "  \"source_files\": [\"main.cpp\", \"src\"],\n";
    file << "  \"include_dirs\": [\"include\", \"/usr/local/include\"],\n";
    file << "  \"library_dirs\": [\"lib\", \"/usr/local/lib\"],\n";
    file << "  \"libraries\": [\"pthread\", \"curl\", \"ssl\", \"crypto\"],\n";
    file << "  \"compile_flags\": [\"-Wall\", \"-Wextra\"],\n";
    file << "  \"cpp_standard\": \"c++17\",\n";
    file << "  \"optimization\": \"O2\"\n";
    file << "}\n";
    file << "```\n\n";
    
    file << "### Example 5: Shared Library\n\n";
    file << "```json\n";
    file << "{\n";
    file << "  \"project_name\": \"mylib\",\n";
    file << "  \"output_type\": \"library\",\n";
    file << "  \"source_files\": [\"src\"],\n";
    file << "  \"include_dirs\": [\"include\"],\n";
    file << "  \"compile_flags\": [\"-fPIC\", \"-Wall\"],\n";
    file << "  \"cpp_standard\": \"c++17\",\n";
    file << "  \"optimization\": \"O3\"\n";
    file << "}\n";
    file << "```\n\n";
    
    file << "### Example 6: Debug Build\n\n";
    file << "```json\n";
    file << "{\n";
    file << "  \"project_name\": \"debugapp\",\n";
    file << "  \"source_files\": [\"main.cpp\", \"src\"],\n";
    file << "  \"include_dirs\": [\"include\"],\n";
    file << "  \"debug\": true,\n";
    file << "  \"optimization\": \"O0\",\n";
    file << "  \"compile_flags\": [\"-Wall\", \"-Wextra\", \"-Werror\"]\n";
    file << "}\n";
    file << "```\n\n";
    
    file << "## Commands\n\n";
    file << "### Initialize Project\n";
    file << "```bash\n";
    file << "./buildhelper init\n";
    file << "```\n";
    file << "Creates `build.json` and `BUILD_GUIDE.md` in the current directory.\n\n";
    
    file << "### Build Project\n";
    file << "```bash\n";
    file << "./buildhelper build\n";
    file << "# or simply\n";
    file << "./buildhelper\n";
    file << "```\n";
    file << "Compiles the project using incremental compilation.\n\n";
    
    file << "### Clean Build Artifacts\n";
    file << "```bash\n";
    file << "./buildhelper clean\n";
    file << "```\n";
    file << "Removes the build directory and all compiled files.\n\n";
    
    file << "### Rebuild Project\n";
    file << "```bash\n";
    file << "./buildhelper rebuild\n";
    file << "```\n";
    file << "Cleans and then builds the project.\n\n";
    
    file << "### Verbose Output\n";
    file << "```bash\n";
    file << "./buildhelper -v build\n";
    file << "```\n";
    file << "Shows detailed configuration information before building.\n\n";
    
    file << "### Custom Config File\n";
    file << "```bash\n";
    file << "./buildhelper myconfig.json\n";
    file << "```\n";
    file << "Uses a custom configuration file.\n\n";
    
    file << "### Help\n";
    file << "```bash\n";
    file << "./buildhelper --help\n";
    file << "```\n\n";
    
    file << "## Tips and Best Practices\n\n";
    file << "1. **Use Directory Scanning:** Instead of listing every file, use directory paths\n";
    file << "2. **Separate Configs:** Create `build-debug.json` and `build-release.json` for different builds\n";
    file << "3. **Enable Warnings:** Use `-Wall -Wextra` during development\n";
    file << "4. **Optimization Levels:** Use `O0` for development, `O2`/`O3` for release\n";
    file << "5. **Debug Symbols:** Enable `debug: true` during development\n\n";
    
    file << "## Troubleshooting\n\n";
    file << "**Problem:** \"Cannot open config file: build.json\"  \n";
    file << "**Solution:** Run `./buildhelper init` to create a default configuration.\n\n";
    file << "**Problem:** \"source_files is required in config file\"  \n";
    file << "**Solution:** Add at least one source file or directory to the `source_files` array.\n\n";
    file << "**Problem:** Directory scanning finds no files  \n";
    file << "**Solution:** Ensure files have correct extensions (`.cpp`, `.cc`, `.cxx`, `.c++`, `.C`) and directory exists.\n\n";
    file << "**Problem:** Linking fails with \"undefined reference\"  \n";
    file << "**Solution:** Add missing libraries to the `libraries` array and verify `library_dirs` paths.\n\n";
    
    file << "---\n\n";
    file << "**Generated by C++ Build Helper**\n";
    
    file.close();
    return true;
}

// 检查文件是否为C++源文件
bool ConfigParser::isCppFile(const std::string& filename) {
    std::vector<std::string> extensions = {".cpp", ".cc", ".cxx", ".c++", ".C"};
    for (const auto& ext : extensions) {
        if (filename.length() >= ext.length() &&
            filename.substr(filename.length() - ext.length()) == ext) {
            return true;
        }
    }
    return false;
}

// 检查路径是否为目录
bool ConfigParser::isDirectory(const std::string& path) {
#ifdef _WIN32
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) return false;
    return (info.st_mode & _S_IFDIR) != 0;
#else
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return false;
    return S_ISDIR(info.st_mode);
#endif
}

// 列出目录中的所有C++文件
std::vector<std::string> ConfigParser::listCppFiles(const std::string& directory) {
    std::vector<std::string> files;
    
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    std::string searchPath = directory + "\\*";
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filename = findData.cFileName;
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (isCppFile(filename)) {
                    files.push_back(directory + "/" + filename);
                }
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (entry->d_type == DT_REG && isCppFile(filename)) {
                files.push_back(directory + "/" + filename);
            }
        }
        closedir(dir);
    }
#endif
    
    return files;
}

// 展开source_files中的目录项
std::vector<std::string> ConfigParser::expandSourceFiles(const std::vector<std::string>& entries) {
    std::vector<std::string> result;
    
    for (const auto& entry : entries) {
        if (isDirectory(entry)) {
            std::cout << "Scanning directory: " << entry << std::endl;
            std::vector<std::string> files = listCppFiles(entry);
            std::cout << "  Found " << files.size() << " C++ files" << std::endl;
            result.insert(result.end(), files.begin(), files.end());
        } else {
            result.push_back(entry);
        }
    }
    
    return result;
}
