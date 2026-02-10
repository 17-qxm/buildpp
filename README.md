# C++ Build Helper

一个简单易用的C++编译辅助工具，比CMake更加简便，支持增量编译、库管理和灵活的配置选项。

## 特性

- **简单配置**: 使用JSON格式的配置文件，易于理解和编辑
- **增量编译**: 自动检测文件修改，只重新编译改变的源文件
- **库管理**: 支持静态库和动态库的链接
- **头文件路径管理**: 轻松管理多个include目录
- **编译选项配置**: 支持C++标准、优化级别、调试选项等
- **跨平台**: 支持Windows和Linux

## 快速开始

### 1. 编译构建工具本身

```bash
g++ -std=c++17 -O2 main.cpp config.cpp compiler.cpp dependency.cpp -o buildhelper
```

Windows:
```bash
g++ -std=c++17 -O2 main.cpp config.cpp compiler.cpp dependency.cpp -o buildhelper.exe
```

### 2. 创建配置文件

创建 `build.json` 文件：

```json
{
  "project_name": "my_project",
  "output_name": "myapp",
  "output_type": "executable",
  "compiler": "g++",
  "cpp_standard": "c++17",
  "optimization": "O2",
  "debug": false,
  "build_dir": "build",
  
  "source_files": [
    "src/main.cpp",
    "src/utils.cpp"
  ],
  
  "include_dirs": [
    "include"
  ],
  
  "library_dirs": [
    "lib"
  ],
  
  "libraries": [
    "pthread"
  ],
  
  "compile_flags": [
    "-Wall",
    "-Wextra"
  ],
  
  "link_flags": []
}
```

### 3. 构建项目

```bash
./buildhelper build
```

## 使用方法

### 基本命令

```bash
# 构建项目（默认）
./buildhelper
./buildhelper build

# 清理构建文件
./buildhelper clean

# 重新构建（清理后构建）
./buildhelper rebuild

# 使用自定义配置文件
./buildhelper myconfig.json

# 显示详细配置信息
./buildhelper -v build

# 显示帮助
./buildhelper --help
```

## 配置文件说明

### 必需字段

- `project_name`: 项目名称
- `source_files`: 源文件列表（数组）

### 可选字段

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `output_name` | string | project_name | 输出文件名 |
| `output_type` | string | "executable" | 输出类型："executable" 或 "library" |
| `compiler` | string | "g++" | 编译器：g++ 或 gcc |
| `cpp_standard` | string | "c++17" | C++标准：c++11, c++14, c++17, c++20等 |
| `optimization` | string | "O2" | 优化级别：O0, O1, O2, O3, Os |
| `debug` | boolean | false | 是否包含调试信息 |
| `build_dir` | string | "build" | 构建目录 |
| `include_dirs` | array | [] | 头文件搜索路径 |
| `library_dirs` | array | [] | 库文件搜索路径 |
| `libraries` | array | [] | 要链接的库（不含-l前缀） |
| `compile_flags` | array | [] | 额外的编译标志 |
| `link_flags` | array | [] | 额外的链接标志 |

## 配置示例

### 示例1: 简单的可执行程序

```json
{
  "project_name": "hello",
  "source_files": ["main.cpp"],
  "cpp_standard": "c++17",
  "optimization": "O2"
}
```

### 示例2: 带库依赖的项目

```json
{
  "project_name": "myapp",
  "output_name": "app",
  "source_files": [
    "src/main.cpp",
    "src/network.cpp",
    "src/database.cpp"
  ],
  "include_dirs": [
    "include",
    "/usr/local/include"
  ],
  "library_dirs": [
    "lib",
    "/usr/local/lib"
  ],
  "libraries": [
    "pthread",
    "curl",
    "sqlite3"
  ],
  "compile_flags": ["-Wall", "-Wextra"],
  "cpp_standard": "c++17",
  "debug": true
}
```

### 示例3: 编译共享库

```json
{
  "project_name": "mylib",
  "output_name": "mylib",
  "output_type": "library",
  "source_files": [
    "src/lib_core.cpp",
    "src/lib_utils.cpp"
  ],
  "include_dirs": ["include"],
  "compile_flags": ["-fPIC"],
  "cpp_standard": "c++17",
  "optimization": "O3"
}
```

## 工作原理

1. **配置解析**: 读取JSON配置文件，解析所有构建参数
2. **依赖检测**: 比较源文件和目标文件的修改时间
3. **增量编译**: 只编译修改过的源文件
4. **链接**: 将所有目标文件链接成最终的可执行文件或库

## 优势对比

与CMake相比：

- 配置文件更简单直观
- 无需学习复杂的CMake语法
- 适合中小型项目快速构建
- 单个可执行文件，无需安装

## 限制

- 不支持复杂的依赖关系管理
- 不支持子项目和模块化构建
- 头文件依赖检测较简单（仅基于时间戳）

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！
