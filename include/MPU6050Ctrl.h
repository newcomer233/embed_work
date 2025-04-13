
#ifndef MPU6050_CTRL_H
#define MPU6050_CTRL_H

#include "MPU6050.h"

// USER interface for callback
class MPU6050DataCallback {
public:
    virtual void onMPU6050Data(const MPU6050_Data& d) = 0;
};

// controller class as a bridge between the user and the MPU6050
class MPU6050Ctrl : public MPU6050EventInterface {
public:
    MPU6050Ctrl(int gpio_line, const char* i2c_dev, int addr);
    ~MPU6050Ctrl();

    bool init();
    void shutdown();

    void setCallback(MPU6050DataCallback* cb);

    // hardware INT
    void onDataReady(const MPU6050_Data& d) override;

private:
    MPU6050 sensor_;
    MPU6050DataCallback* callback_;
};

#endif
