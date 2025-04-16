cmake_minimum_required(VERSION 3.13)
project(vosk_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 头文件目录
include_directories(
	include
	/home/newcomer233/Desktop/embed_final_v09/lib/onnxruntime-linux-aarch64-1.21.0/include
	/home/newcomer233/Desktop/embed_final_v09/lib/Linux_aarch64/piper_phonemize/include
	/home/newcomer233/Desktop/embed_final_v09/lib/si/include
)

# 动态库目录
link_directories(
	lib
	/home/newcomer233/Desktop/embed_final_v09/lib/onnxruntime-linux-aarch64-1.21.0/lib
	/home/newcomer233/Desktop/embed_final_v09/lib/Linux_aarch64/piper_phonemize/lib
	/home/newcomer233/Desktop/embed_final_v09/lib/si/lib
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
