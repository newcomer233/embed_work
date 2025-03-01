#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "paj7620.h"

#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_PIN 17  // 监听 GPIO 17 中断

std::mutex mtx;
std::condition_variable cv;
bool gesture_ready = false;

void gpio_interrupt_thread() {
    std::cout << "GPIO Interrupt Listening..." << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 模拟中断事件
        std::cout << "Interrupt detected! Waking up gesture reader." << std::endl;
        {
            std::lock_guard<std::mutex> lock(mtx);
            gesture_ready = true;
        }
        cv.notify_one();
    }
}

void gesture_read_thread(PAJ7620& sensor) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return gesture_ready; });

        // 读取手势
        std::string gesture = sensor.getGesture();
        if (gesture != "None") {
            std::cout << "Detected Gesture: " << gesture << std::endl;
        }

        gesture_ready = false;
    }
}

int main() {
    PAJ7620 sensor("/dev/i2c-3");

    if (!sensor.initialize()) {
        std::cerr << "PAJ7620 initialization failed" << std::endl;
        return -1;
    }

    std::thread gpio_thread(gpio_interrupt_thread);
    std::thread gesture_thread(gesture_read_thread, std::ref(sensor));

    gpio_thread.join();
    gesture_thread.join();

    return 0;
}
