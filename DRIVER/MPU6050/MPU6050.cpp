// MPU6050.cpp
#include "MPU6050.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <thread>
#include <fstream>
#include <poll.h>

MPU6050::MPU6050(uint8_t address, int intPin) : i2cAddr(address), intPin(intPin), dataReady(false),
    prev_ax(0), prev_ay(0), prev_az(0), prev_gx(0), prev_gy(0), prev_gz(0) {
    
    const char* i2cDev = "/dev/i2c-1";
    i2cFile = open(i2cDev, O_RDWR);
    if (i2cFile < 0) {
        std::cerr << "Failed to open I2C device" << std::endl;
        exit(1);
    }

    if (ioctl(i2cFile, I2C_SLAVE, i2cAddr) < 0) {
        std::cerr << "Failed to connect to MPU6050" << std::endl;
        exit(1);
    }
}

MPU6050::~MPU6050() {
    close(i2cFile);
}

void MPU6050::writeByte(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    write(i2cFile, buffer, 2);
}

int16_t MPU6050::readWord(uint8_t reg) {
    uint8_t buffer[2] = {0};
    write(i2cFile, &reg, 1);
    read(i2cFile, buffer, 2);
    return (int16_t)((buffer[0] << 8) | buffer[1]);
}

void MPU6050::initialize() {
    writeByte(0x6B, 0x00);
    writeByte(0x37, 0x00);
    writeByte(0x38, 0x01);
}

void MPU6050::readRawData(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz) {
    ax = readWord(0x3B);
    ay = readWord(0x3D);
    az = readWord(0x3F);
    gx = readWord(0x43);
    gy = readWord(0x45);
    gz = readWord(0x47);
}

void MPU6050::readDiffData(int16_t &dax, int16_t &day, int16_t &daz, int16_t &dgx, int16_t &dgy, int16_t &dgz) {
    int16_t ax, ay, az, gx, gy, gz;
    readRawData(ax, ay, az, gx, gy, gz);
    dax = ax - prev_ax;
    day = ay - prev_ay;
    daz = az - prev_az;
    dgx = gx - prev_gx;
    dgy = gy - prev_gy;
    dgz = gz - prev_gz;
    prev_ax = ax;
    prev_ay = ay;
    prev_az = az;
    prev_gx = gx;
    prev_gy = gy;
    prev_gz = gz;
}

void MPU6050::handleInterrupt() {
    std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(intPin) + "/value";
    int gpio_fd = open(gpioPath.c_str(), O_RDONLY);
    if (gpio_fd < 0) {
        std::cerr << "Failed to open GPIO" << std::endl;
        return;
    }

    struct pollfd pfd;
    pfd.fd = gpio_fd;
    pfd.events = POLLPRI;

    char buf[10];
    while (true) {
        poll(&pfd, 1, -1);
        lseek(gpio_fd, 0, SEEK_SET);
        read(gpio_fd, buf, sizeof(buf));
        if (buf[0] == '1') {
            {
                std::lock_guard<std::mutex> lock(mtx);
                dataReady = true;
            }
            cv.notify_one();
        }
    }
    close(gpio_fd);
}
