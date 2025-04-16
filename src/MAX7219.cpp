#include "MAX7219.h"
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <cstring>
#include <thread>

#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_SPEED 1000000

MAX7219::MAX7219() {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        std::cerr << "无法打开 SPI 设备: " << SPI_DEVICE << std::endl;
        exit(1);
    }

    uint8_t mode = 0;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    memset(devicePatterns, 0, sizeof(devicePatterns));
    initialize();
}

MAX7219::~MAX7219() {
    clearDisplayAll();
    close(spi_fd);
}

void MAX7219::initialize() {
    writeRegister(0x09, 0x00, 0x09, 0x00);
    writeRegister(0x0A, 0x00, 0x0A, 0x00);
    writeRegister(0x0B, 0x07, 0x0B, 0x07);
    writeRegister(0x0C, 0x01, 0x0C, 0x01);
    writeRegister(0x0F, 0x00, 0x0F, 0x00);
    clearDisplayAll();
}

void MAX7219::clearDisplayAll() {
    for (int i = 1; i <= 8; i++) {
        sendData(0, i, 0x00);
        sendData(1, i, 0x00);
    }
}

void MAX7219::clearDisplay() {
    clearDisplayAll();
}

void MAX7219::setBrightness(uint8_t brightness) {
    if (brightness > 15) brightness = 15;
    writeRegister(0x0A, brightness, 0x0A, brightness);
}

void MAX7219::sendData(int device, uint8_t reg, uint8_t value) {
    if (device < 0 || device > 1) return;
    devicePatterns[device][reg - 1] = value;
    writeRegister(reg, devicePatterns[0][reg - 1], reg, devicePatterns[1][reg - 1]);
}

void MAX7219::writeRegister(uint8_t reg0, uint8_t value0, uint8_t reg1, uint8_t value1) {
    uint8_t tx_buffer[4] = {reg0, value0, reg1, value1};
    write(spi_fd, tx_buffer, sizeof(tx_buffer));
}

void MAX7219::displayPattern(int device, const uint8_t pattern[8]) {
    if (device < 0 || device > 1) return;
    for (int i = 0; i < 8; ++i) {
        sendData(device, i + 1, pattern[i]);
    }
}

void MAX7219::displayTwoDigits(int number, int device) {
    if (number < 0 || number > 99) return;

    int tens = number / 10;
    int units = number % 10;

    uint8_t merged[8];
    for (int i = 0; i < 8; ++i) {
        uint8_t left = Patterns::digitPatterns[tens][i] >> 4;
        uint8_t right = Patterns::digitPatterns[units][i] >> 4;
        merged[i] = (left << 4) | right;
        sendData(device, i + 1, merged[i]);
    }
}

void MAX7219::displayTime(int hour, int minute) {
    int hourTens = hour / 10;
    int hourUnits = hour % 10;
    int minuteTens = minute / 10;
    int minuteUnits = minute % 10;

    uint8_t hourMerged[8];
    uint8_t minuteMerged[8];

    for (int i = 0; i < 8; ++i) {
        uint8_t left = Patterns::digitPatterns[hourTens][i] >> 4;
        uint8_t right = Patterns::digitPatterns[hourUnits][i] >> 4;
        hourMerged[i] = (left << 4) | right;

        uint8_t mLeft = Patterns::digitPatterns[minuteTens][i] >> 4;
        uint8_t mRight = Patterns::digitPatterns[minuteUnits][i] >> 4;
        minuteMerged[i] = ((mLeft << 4) | mRight) >> 1; // 秒右移一格

        writeRegister(i + 1, hourMerged[i], i + 1, minuteMerged[i]);
    }
}

void MAX7219::displayTemperature(int temp, int device) {
    if (temp >= 0 && temp <= 9) {
        // 个位数：右移数字 + 加摄氏度
        for (int i = 0; i < 8; ++i) {
            uint8_t digit = Patterns::digitPatterns[temp][i] >> 4;     // 取左4列
            digit >>= 1;                                               // 再右移一位
            uint8_t cmark = Patterns::Celcius4x8[i] >> 4;              // 摄氏度图案
            uint8_t row = (digit << 4) | cmark;                        // 合并
            sendData(device, i + 1, row);
        }
    } else if (temp >= 10 && temp <= 99) {
        int tens = temp / 10;
        int units = temp % 10;
    
        for (int i = 0; i < 8; ++i) {
            uint8_t left = Patterns::digitPatterns[tens][i] >> 4;
            uint8_t right = Patterns::digitPatterns[units][i] >> 4;
            uint8_t row = (left << 4) | right;
            row >>= 1;  
            sendData(device, i + 1, row);
        }
    } else if (temp < 0 && temp >= -9) {
        int absVal = -temp;
    for (int i = 0; i < 8; ++i) {
        uint8_t left = Patterns::Minus4x8[i] >> 4;
        uint8_t right = Patterns::digitPatterns[absVal][i] >> 4;
        uint8_t row = (left << 4) | right;

        row >>= 1;  
        sendData(device, i + 1, row);
        }
    } else {
        clearDisplayAll(); 
    }
}

void MAX7219::displayTimeWithColon(int hour, int minute, bool colonVisible) {
    if (hour < 0 || hour > 99 || minute < 0 || minute > 99) return;

    int hourTens = hour / 10;
    int hourUnits = hour % 10;
    int minuteTens = minute / 10;
    int minuteUnits = minute % 10;

    uint8_t hourMerged[8];
    uint8_t minuteMerged[8];

    for (int i = 0; i < 8; ++i) {
        uint8_t hourLeft = Patterns::digitPatterns[hourTens][i] >> 4;
        uint8_t hourRight = Patterns::digitPatterns[hourUnits][i] >> 4;
        hourMerged[i] = (hourLeft << 4) | hourRight;

        // 分钟部分，左移一位去掉末尾列
        uint8_t minuteLeft = Patterns::digitPatterns[minuteTens][i] >> 4;
        uint8_t minuteRight = Patterns::digitPatterns[minuteUnits][i] >> 4;
        minuteMerged[i] = ((minuteLeft << 4) | minuteRight) >> 1;

        // 根据冒号可见性设置冒号
        if (colonVisible) {
            if (i == 4) hourMerged[i] |= 0b00000001;   // 小时模块冒号上点
            if (i == 3) minuteMerged[i] |= 0b10000000; // 分钟模块冒号下点
        } else {
            if (i == 4) hourMerged[i] &= 0b11111110;   // 清除小时模块冒号上点
            if (i == 3) minuteMerged[i] &= 0b01111111; // 清除分钟模块冒号下点
        }

        // 更新缓存并发送数据
        devicePatterns[0][i] = hourMerged[i];
        devicePatterns[1][i] = minuteMerged[i];

        writeRegister(i + 1, devicePatterns[0][i], i + 1, devicePatterns[1][i]);
    }
}

void MAX7219::blinkWeatherWithTemp(const uint8_t pattern1[8], const uint8_t pattern2[8], int temp, int duration_ms, int interval_ms) {
    int cycles = duration_ms / (2 * interval_ms);
    for (int i = 0; i < cycles; ++i) {
        displayPattern(0, pattern1);
        displayTemperature(temp, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));

        displayPattern(0, pattern2);
        displayTemperature(temp, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
    displayPattern(0, pattern1);
    displayTemperature(temp, 1);
}

// for snake game
void MAX7219::setPixel(int x, int y, bool on) {
    if (flipX) x = 15 - x;
    if (flipY) y = 7 - y;
    if (x < 0 || x >= 16 || y < 0 || y >= 8) return;

    int dev = x / 8;
    int col = x % 8;

    if (on)
        devicePatterns[dev][y] |= (1 << col);
    else
        devicePatterns[dev][y] &= ~(1 << col);
}


void MAX7219::refresh() {
    for (int row = 0; row < 8; ++row) {
        uint8_t tx[4];
        tx[0] = row + 1;
        tx[1] = devicePatterns[1][row];  // 右边设备（显示左边）
        tx[2] = row + 1;
        tx[3] = devicePatterns[0][row];  // 左边设备（显示右边）
        write(spi_fd, tx, 4);
    }
}

// void MAX7219::sendData(int device, uint8_t reg, uint8_t data) {
//     uint8_t tx[2 * numDevices] = {0};
//     tx[(numDevices - 1 - device) * 2] = reg;
//     tx[(numDevices - 1 - device) * 2 + 1] = data;
//     write(spi_fd, tx, 2 * numDevices);
// }