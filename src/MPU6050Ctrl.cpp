
#include "MPU6050Ctrl.h"
#include <iostream>

MPU6050Ctrl::MPU6050Ctrl(int gpio_line, const char* i2c_dev, int addr)
    : sensor_(gpio_line, i2c_dev, addr), callback_(nullptr) {}

MPU6050Ctrl::~MPU6050Ctrl() {
    shutdown();
}

bool MPU6050Ctrl::init() {
    if (!sensor_.init()) {
        std::cerr << "[MPU6050Ctrl] 传感器初始化失败\n";
        return false;
    }

    sensor_.registerCallback(this);  // 注册中断回调
    std::cout << "[MPU6050Ctrl] 成功注册为底层中断监听者\n";
    return true;
}

void MPU6050Ctrl::shutdown() {
    sensor_.close();
}

void MPU6050Ctrl::setCallback(MPU6050DataCallback* cb) {
    callback_ = cb;
}

void MPU6050Ctrl::onDataReady(const MPU6050_Data& d) {
    if (callback_) {
        callback_->onMPU6050Data(d);  // 转发给外部用户注册的回调
    }
}
