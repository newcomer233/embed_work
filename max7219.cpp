#include "max7219.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdexcept>

Max7219::Max7219(int devices) : numDevices(devices) {
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) throw std::runtime_error("can't open SPI device");

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 1000000;

    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    std::memset(buffer, 0, sizeof(buffer));
    init();
}

Max7219::~Max7219() {
    if (spi_fd >= 0) close(spi_fd);
}

void Max7219::init() {
    sendAll(0x0F, 0x00);
    sendAll(0x0C, 0x01);
    sendAll(0x0B, 0x07);
    sendAll(0x0A, 0x00); // lowest brightness
    sendAll(0x09, 0x00);
    clear();
    refresh();
}

void Max7219::clear() {
    std::memset(buffer, 0, sizeof(buffer));
}

void Max7219::setPixel(int x, int y, bool on) {
    x = numDevices * 8 - 1 - x;

    if (x < 0 || x >= numDevices * 8 || y < 0 || y >= 8) return;
    int dev = x / 8;
    int col = x % 8;
    if (on)
        buffer[y][dev] |= (1 << col);
    else
        buffer[y][dev] &= ~(1 << col);
}


void Max7219::refresh() {
    for (int row = 0; row < 8; ++row) {
        uint8_t tx[2 * numDevices];
        for (int dev = 0; dev < numDevices; ++dev) {
            tx[dev * 2] = row + 1;
            tx[dev * 2 + 1] = buffer[row][numDevices - 1 - dev];
        }
        write(spi_fd, tx, 2 * numDevices);
    }
}

void Max7219::sendData(int device, uint8_t reg, uint8_t data) {
    uint8_t tx[2 * numDevices] = {0};
    tx[(numDevices - 1 - device) * 2] = reg;
    tx[(numDevices - 1 - device) * 2 + 1] = data;
    write(spi_fd, tx, 2 * numDevices);
}

void Max7219::sendAll(uint8_t reg, uint8_t data) {
    for (int i = 0; i < numDevices; ++i) {
        sendData(i, reg, data);
    }
}
