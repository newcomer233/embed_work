#include "PAJ7620.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define PAJ7620U2_ADDR 0x73
#define PAJ7620U2_REG_GESTURE 0x43
#define PAJ7620U2_REG_BANK_SEL 0xEF

const char* GESTURES[] = {
    "None", "Right", "Left", "Up", "Down", "Forward",
    "Backward", "Clockwise", "CounterClockwise"
};

PAJ7620::PAJ7620(const std::string& i2c_device) : i2c_device_path(i2c_device) {
    i2c_fd = open(i2c_device.c_str(), O_RDWR);
    if (i2c_fd < 0) {
        std::cerr << "Error: Invalid I2C device path " << i2c_device_path << std::endl;
        return false;
    } else {
        if (ioctl(i2c_fd, I2C_SLAVE, PAJ7620U2_ADDR) < 0) {
            std::cerr << "Error: Cannot set I2C address" << std::endl;
            close(i2c_fd);
            i2c_fd = -1;
        }
    }
}

PAJ7620::~PAJ7620() {
    if (i2c_fd >= 0) {
        close(i2c_fd);
    }
}

bool PAJ7620::initialize() {
    if (i2c_fd < 0) return false;
    return writeRegister(PAJ7620U2_REG_BANK_SEL, 0x00);
}

std::string PAJ7620::getGesture() {
    if (i2c_fd < 0) return "Error";
    
    uint8_t gesture;
    if (!readRegister(PAJ7620U2_REG_GESTURE, gesture)) {
        return "Error";
    }

    if (gesture == 0x00) return "None";
    if (gesture >= 0x01 && gesture <= 8) return GESTURES[gesture];

    return "Unknown";
}

bool PAJ7620::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    return write(i2c_fd, buffer, 2) == 2;
}

bool PAJ7620::readRegister(uint8_t reg, uint8_t &value) {
    if (write(i2c_fd, &reg, 1) != 1) return false;
    return read(i2c_fd, &value, 1) == 1;
}