#include <iostream>
#include "MPU6050.h"
#include <unistd.h>

int main() {
    MPU6050 mpu;
    mpu.initialize();

    int16_t ax, ay, az, gx, gy, gz;

    while (true) {
        mpu.readRawData(ax, ay, az, gx, gy, gz);
        std::cout << "Accel: X=" << ax << " Y=" << ay << " Z=" << az
                  << " | Gyro: X=" << gx << " Y=" << gy << " Z=" << gz << std::endl;

        usleep(500000);  // 500ms 延迟
    }

    return 0;
}
