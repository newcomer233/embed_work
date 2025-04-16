
#ifndef MPU6050_CTRL_H
#define MPU6050_CTRL_H

#include "MPU6050.h"

// 用户注册的上层回调接口（非底层直接对接接口）
class MPU6050DataCallback {
public:
    virtual void onMPU6050Data(const MPU6050_Data& d) = 0;
};

// 控制类，继承底层事件接口，起桥接作用
class MPU6050Ctrl : public MPU6050EventInterface {
public:
    MPU6050Ctrl(int gpio_line, const char* i2c_dev, int addr);
    ~MPU6050Ctrl();

    bool init();
    void shutdown();

    void setCallback(MPU6050DataCallback* cb);

    // 底层中断接口实现
    void onDataReady(const MPU6050_Data& d) override;

private:
    MPU6050 sensor_;
    MPU6050DataCallback* callback_;
};

#endif
