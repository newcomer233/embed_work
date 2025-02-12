#include "MPU6050.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

MPU6050::MPU6050(uint8_t address) : i2cAddr(address) {
    const char* i2cDev = "/dev/i2c-1";  // I2C 设备文件
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

    return (int16_t)((buffer[0] << 8) | buffer[1]);  // 高字节在前，低字节在后
}

void MPU6050::initialize() {
    writeByte(0x6B, 0x00);  // 解除休眠模式
}

void MPU6050::readRawData(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz) {
    ax = readWord(0x3B);
    ay = readWord(0x3D);
    az = readWord(0x3F);
    gx = readWord(0x43);
    gy = readWord(0x45);
    gz = readWord(0x47);
}
