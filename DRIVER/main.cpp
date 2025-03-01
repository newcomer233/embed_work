// the library or define we need
    #include <iostream>
    #include <thread>
    #include "MPU6050.h"
    #include "PAJ7620.h"

    #define GPIO_CHIP "/dev/gpiochip0"
    #define GPIO_PIN 27  // 监听 PAJ7620U2 手势中断

// the class we need to define
    // the MPU6050
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
    
    // the PAJ7620
    void gestureInterruptThread(PAJ7620 &sensor) {
        gpiod_chip *chip = gpiod_chip_open(GPIO_CHIP);
        if (!chip) {
            std::cerr << "Error: Cannot open GPIO chip" << std::endl;
            return;
        }
    
        gpiod_line *line = gpiod_chip_get_line(chip, GPIO_PIN);
        if (!line) {
            std::cerr << "Error: Cannot get GPIO line" << std::endl;
            gpiod_chip_close(chip);
            return;
        }
    
        if (gpiod_line_request_falling_edge_events(line, "gesture_interrupt") < 0) {
            std::cerr << "Error: Cannot request GPIO events" << std::endl;
            gpiod_chip_close(chip);
            return;
        }
    
        std::cout << "PAJ7620U2 GPIO Interrupt Listening..." << std::endl;
        struct timespec timeout = { 2, 0 }; // 超时时间 2 秒
        
        while (true) {
            struct gpiod_line_event event;
            int ret = gpiod_line_event_wait(line, &timeout);
            if (ret > 0) {
                gpiod_line_event_read(line, &event);
                std::cout << "PAJ7620U2 Interrupt detected! Waking up gesture reader." << std::endl;
    
                std::lock_guard<std::mutex> lock(sensor.mtx);
                sensor.dataReady = true;
                sensor.cv.notify_one();
            }
        }
        gpiod_chip_close(chip);
    }
    
    void gestureDataThread(PAJ7620 &sensor) {
        while (true) {
            std::unique_lock<std::mutex> lock(sensor.mtx);
            sensor.cv.wait(lock, [&] { return sensor.dataReady.load(); });
    
            std::string gesture = sensor.getGesture();
            if (gesture != "None") {
                std::cout << "Detected Gesture: " << gesture << std::endl;
            }
            sensor.dataReady = false;
        }
    }
    
//  main function
int main() {
    MPU6050 mpu(0x68, 17);
    PAJ7620 sensor("/dev/i2c-3");

    if (!sensor.initialize()) {
        std::cerr << "PAJ7620U2 initialization failed" << std::endl;
        return -1;
    }

    std::thread t1(interruptThread, std::ref(mpu));
    std::thread t2(dataThread, std::ref(mpu));
    std::thread t3(gestureInterruptThread, std::ref(sensor));
    std::thread t4(gestureDataThread, std::ref(sensor));
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    return 0;
}