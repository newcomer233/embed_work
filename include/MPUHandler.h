#pragma once
#include "MPU6050Ctrl.h"
#include "AppEvent.h"
#include <functional>
#include <iostream>

class MPUHandler : public MPU6050DataCallback {
public:
    using EventCallback = std::function<void(AppEvent)>;

    MPUHandler(EventCallback cb) : eventCallback(cb) {}

    void onMPU6050Data(const MPU6050_Data&) override {
        std::cout << "[MPU INT]  SwitchMode triggered" << std::endl;
        eventCallback(AppEvent::SwitchMode);
    }

private:
    EventCallback eventCallback;
};