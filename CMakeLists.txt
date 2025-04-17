cmake_minimum_required(VERSION 3.13)
project(vosk_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(
	include
	${CMAKE_SOURCE_DIR}/lib/onnxruntime-linux-aarch64-1.21.0/include
	${CMAKE_SOURCE_DIR}/lib/Linux_aarch64/piper_phonemize/include
	${CMAKE_SOURCE_DIR}/lib/si/include
)

# 动态库目录
link_directories(
	lib
	${CMAKE_SOURCE_DIR}/lib/onnxruntime-linux-aarch64-1.21.0/lib
	${CMAKE_SOURCE_DIR}/lib/Linux_aarch64/piper_phonemize/lib
	${CMAKE_SOURCE_DIR}/lib/si/lib
)

# 源文件
file(GLOB SOURCES "src/*.cpp")

# 可执行文件
add_executable(embed_demo ${SOURCES})

# 链接 vosk 库
target_link_libraries(embed_demo 
	vosk 
	gpiod 
	curl 
	jsoncpp
	onnxruntime
	piper_phonemize
	espeak-ng
	spdlog
)
