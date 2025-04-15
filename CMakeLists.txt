cmake_minimum_required(VERSION 3.13)
project(vosk_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(include)

# 动态库目录
link_directories(lib)

# 设置输出路径（可选）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 源文件
file(GLOB SOURCES "src/*.cpp")

# 可执行文件
add_executable(vosk_demo ${SOURCES})

# 链接 vosk 库
target_link_libraries(vosk_demo vosk)
