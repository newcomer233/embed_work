#ifndef MPU6050_H
#define MPU6050_H

#include <cstdint>

class MPU6050 {
private:
    int i2cFile;
    uint8_t i2cAddr;

    int16_t readWord(uint8_t reg);
    void writeByte(uint8_t reg, uint8_t value);

public:
    explicit MPU6050(uint8_t address = 0x68);
    ~MPU6050();

    void initialize();
    void readRawData(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz);
};

#endif
