#ifndef MPU6050_H
#define MPU6050_H

#include <cstdint>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
extern "C" {
    #include <gpiod.h>
}
struct MPU6050_Data {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    float temp;
};

class MPU6050EventInterface {
public:
    virtual void onDataReady(const MPU6050_Data& data) = 0;
    virtual ~MPU6050EventInterface() = default;
};

class MPU6050 {
public:
    MPU6050(int gpio_line = 13, const char* i2c_dev = "/dev/i2c-1", int addr = 0x68);
    ~MPU6050();

    bool init();
    void registerCallback(MPU6050EventInterface* cb);
    void close();

    bool getData(MPU6050_Data& out);

private:
    void irqLoop();
    bool readRawData(MPU6050_Data& out);
    bool deviceInit(); // 添加初始化函数声明
 
    int gpio_line_;
    const char* i2c_dev_;
    int i2c_fd_;
    int addr_;

    struct gpiod_chip* chip_;
    struct gpiod_line* line_;
    std::thread irq_thread_;
    std::atomic<bool> running_;
    std::mutex cb_mutex_;
    std::vector<MPU6050EventInterface*> callbacks_;

    std::mutex data_mutex_;
    MPU6050_Data latest_;
};

#endif