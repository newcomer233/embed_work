#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#define SPI_PATH "/dev/spidev0.0"

#define SPI_DEVICE "/dev/spidev0.0"  // 确保使用 SPI0
#define SPI_SPEED 1000000            //6 1 MHz

class MAX7219 {
public:
    MAX7219() {
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

    ~MAX7219() {
        close(spi_fd);
    }

    void initialize() {
        write_register(0x09, 0x00); // 译码模式：无译码
        write_register(0x0A, 0x0F); // 亮度：最大
        write_register(0x0B, 0x07); // 扫描所有 8 行
        write_register(0x0C, 0x01); // 关闭待机模式（开启显示）
        write_register(0x0F, 0x00); // 关闭测试模式
        clear_display();
    }

    void write_register(uint8_t reg, uint8_t value) {
        uint8_t tx_buffer[2] = {reg, value};
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx_buffer,
            .rx_buf = 0,
            .len = 2,
            .speed_hz = SPI_SPEED,
            .delay_usecs = 0,
            .bits_per_word = 8,
        };

        int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1) {
            std::cerr << "❌ SPI 传输失败！（寄存器：" << (int)reg << ", 值：" << (int)value << "）" << std::endl;
        }
    }

    void clear_display() {
        for (int i = 1; i <= 8; i++) {
            write_register(i, 0x00);
        }
    }

    void display_pattern(const uint8_t pattern[8]) {
        for (int i = 0; i < 8; i++) {
            write_register(i + 1, pattern[i]);
        }
    }

private:
    int spi_fd;
};

// 测试代码
int main() {
    MAX7219 max7219;

    // 自定义 8x8 LED 显示图案（爱心图案）
    uint8_t heart_pattern[8] = {
        0b01100110,
        0b11111111,
        0b11111111,
        0b01111110,
        0b00111100,
        0b00011000,
        0b00000000,
        0b00000000
    };

    std::cout << "✅ 显示 8x8 LED 图案！" << std::endl;
    max7219.display_pattern(heart_pattern);

    return 0;
}