#include "MAX7219.h"
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <cstring>

#define SPI_DEVICE "/dev/spidev0.0"  // SPI0 设备
#define SPI_SPEED 1000000            // 1 MHz

MAX7219::MAX7219() {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        std::cerr << "❌ 无法打开 SPI 设备 " << SPI_DEVICE << std::endl;
        exit(1);
    }

    uint8_t mode = 0;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    initialize();
}

MAX7219::~MAX7219() {
    close(spi_fd);
}

void MAX7219::initialize() {
    write_register(0x09, 0x00); // 译码模式：无译码
    set_brightness(15);         // 默认亮度最大
    write_register(0x0B, 0x07); // 扫描所有 8 行
    set_display(true);          // 开启显示
    set_test_mode(false);       // 关闭测试模式
    clear_display();
}

// **🔥 让外部代码也可以调用 write_register()**
void MAX7219::write_register(uint8_t reg, uint8_t value) {
    uint8_t tx_buffer[2] = {reg, value};
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = 0,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .delay_usecs = 0,
        .bits_per_word = 8,
    };

    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}

// **✅ 设置亮度**
void MAX7219::set_brightness(uint8_t level) {
    if (level > 15) level = 15; // 亮度范围 0-15
    write_register(0x0A, level);
}

// **✅ 开启/关闭显示**
void MAX7219::set_display(bool state) {
    write_register(0x0C, state ? 0x01 : 0x00);
}

// **✅ 启用/禁用 BCD 译码模式（用于数码管）**
void MAX7219::set_decode_mode(bool enable) {
    write_register(0x09, enable ? 0xFF : 0x00);
}

// **✅ 进入/退出测试模式**
void MAX7219::set_test_mode(bool enable) {
    write_register(0x0F, enable ? 0x01 : 0x00);
}

void MAX7219::clear_display() {
    memset(buffer, 0, sizeof(buffer));
    update_display();
}

void MAX7219::update_display() {
    for (int i = 0; i < 8; i++) {
        write_register(i + 1, buffer[i]);
    }
}

void MAX7219::set_pixel(uint8_t x, uint8_t y, bool state) {
    if (x >= 8 || y >= 8) return;
    if (state)
        buffer[y] |= (1 << (7 - x));  // 点亮 LED
    else
        buffer[y] &= ~(1 << (7 - x)); // 熄灭 LED
    update_display();
}
