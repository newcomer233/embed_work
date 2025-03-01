#include <iostream>
#include <thread>
#include "MPU6050.h"

void interruptThread(MPU6050 &mpu) {
    mpu.handleInterrupt();
}

void dataThread(MPU6050 &mpu) {
    int16_t dax, day, daz, dgx, dgy, dgz;

    while (true) {
        std::unique_lock<std::mutex> lock(mpu.mtx);
        mpu.cv.wait(lock, [&] { return mpu.dataReady.load(); });

        mpu.readDiffData(dax, day, daz, dgx, dgy, dgz);
        std::cout << "Accel Diff: X=" << dax << " Y=" << day << " Z=" << daz
                  << " | Gyro Diff: X=" << dgx << " Y=" << dgy << " Z=" << dgz << std::endl;

        mpu.dataReady = false;
    }
}

int main() {
    MPU6050 mpu;
    mpu.initialize();

    std::thread t1(interruptThread, std::ref(mpu));
    std::thread t2(dataThread, std::ref(mpu));

    t1.join();
    t2.join();

    return 0;
}
