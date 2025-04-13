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
        std::cerr << "[SensorCtrl] initial PAJ7260U2 fail\n";
        return false;
    }
    paj_->registerCallback(this);
    std::cout << "[SensorCtrl] PAJ7260U2 initial success\n";
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
    std::cout << "[SensorCtrl] gesture detect: 0x" << std::hex << (int)data.gesture << std::dec << "\n";
    if (gestureHandler_) {
        gestureHandler_->handleGesture(data.gesture);
    }
}
