// MPU6050.h
#ifndef MPU6050_H
#define MPU6050_H

#include <cstdint>
#include <atomic>
#include <condition_variable>
#include <mutex>

class MPU6050 {
private:
    int i2cFile;
    uint8_t i2cAddr;
    int intPin; // 绑定的 GPIO 引脚
    std::atomic<bool> dataReady; // 标记中断是否发生
    std::mutex mtx;
    std::condition_variable cv;

    int16_t readWord(uint8_t reg);
    void writeByte(uint8_t reg, uint8_t value);

    int16_t prev_ax, prev_ay, prev_az, prev_gx, prev_gy, prev_gz;

public:
    explicit MPU6050(uint8_t address = 0x68, int intPin = 17);
    ~MPU6050();

    void initialize();
    void readRawData(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz);
    void readDiffData(int16_t &dax, int16_t &day, int16_t &daz, int16_t &dgx, int16_t &dgy, int16_t &dgz);
    void handleInterrupt(); // 处理中断
};

#endif
