#pragma once

#include "PAJ7260U2.h"
#include "GestureHandler.h"
#include <memory>

class SensorCtrl : public Paj7260EventInterface {
public:
    SensorCtrl(int gpio_line, const char* i2c_dev = "/dev/i2c-3", int i2c_addr = 0x73);
    ~SensorCtrl();

    bool init();        // 初始化 I2C + 中断 + PAJ 配置
    void shutdown();    // 清理资源
    void setGestureHandler(GestureHandler* handler); // 设置上层动作处理接口

    // 回调接口实现
    void onGestureDetected(const Paj7260Data& data) override;

private:
    std::unique_ptr<Paj7260U2> paj_;
    GestureHandler* gestureHandler_;  // 非拥有指针，由上层传入
};
