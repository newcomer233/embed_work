
#ifndef MPU6050_CTRL_H
#define MPU6050_CTRL_H

#include "MPU6050.h"

// callback for user
class MPU6050DataCallback {
public:
    virtual void onMPU6050Data(const MPU6050_Data& d) = 0;
};

// bridge
class MPU6050Ctrl : public MPU6050EventInterface {
public:
    MPU6050Ctrl(int gpio_line, const char* i2c_dev, int addr);
    ~MPU6050Ctrl();

    bool init();
    void shutdown();

    void setCallback(MPU6050DataCallback* cb);


    void onDataReady(const MPU6050_Data& d) override;

private:
    MPU6050 sensor_;
    MPU6050DataCallback* callback_;
};

#endif
