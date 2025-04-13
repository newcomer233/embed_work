#pragma once

#include "PAJ7260U2.h"
#include "GestureHandler.h"
#include <memory>

class SensorCtrl : public Paj7260EventInterface {
public:
    SensorCtrl(int gpio_line, const char* i2c_dev = "/dev/i2c-3", int i2c_addr = 0x73);
    ~SensorCtrl();

    bool init();        // INITAL I2C PAJ INT
    void shutdown();    // QUIT AND CLEAR
    void setGestureHandler(GestureHandler* handler); // USER interface to set the gesture handler

    // Callback function for gesture detection
    void onGestureDetected(const Paj7260Data& data) override;

private:
    std::unique_ptr<Paj7260U2> paj_;
    GestureHandler* gestureHandler_;  // User-defined gesture handler
};
