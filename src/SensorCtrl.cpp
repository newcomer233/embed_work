#include "SensorCtrl.h"
#include <iostream>

SensorCtrl::SensorCtrl(int gpio_line, const char* i2c_dev, int i2c_addr)
    : gestureHandler_(nullptr) {
    paj_ = std::make_unique<Paj7260U2>(gpio_line, i2c_dev, i2c_addr);
}

SensorCtrl::~SensorCtrl() {
    shutdown();
}

bool SensorCtrl::init() {
    if (!paj_->init()) {
        std::cerr << "[SensorCtrl] 初始化 PAJ7260U2 失败\n";
        return false;
    }
    paj_->registerCallback(this);
    std::cout << "[SensorCtrl] 初始化完成\n";
    return true;
}

void SensorCtrl::shutdown() {
    if (paj_) {
        paj_->close();
    }
}

void SensorCtrl::setGestureHandler(GestureHandler* handler) {
    gestureHandler_ = handler;
}

void SensorCtrl::onGestureDetected(const Paj7260Data& data) {
    std::cout << "[SensorCtrl] 检测到手势: 0x" << std::hex << (int)data.gesture << std::dec << "\n";
    if (gestureHandler_) {
        gestureHandler_->handleGesture(data.gesture);
    }
}
