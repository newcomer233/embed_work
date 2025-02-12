cmake_minimum_required(VERSION 3.10)
project(MPU6050_example)

set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_CXX_STANDARD 11)

add_executable(MPU6050_example MPU6050_example.cpp MPU6050.cpp)
