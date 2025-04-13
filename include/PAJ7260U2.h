// Paj7260U2.h
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <linux/i2c-dev.h>
extern "C" {
    #include <gpiod.h>
}
#include <cstdint>
#include <atomic>

struct Paj7260Data {
    uint8_t gesture = 0;
    uint8_t proximity = 0;
};

struct Paj7260EventInterface {
    virtual void onGestureDetected(const Paj7260Data& data) = 0;
    virtual ~Paj7260EventInterface() = default;
};

class Paj7260U2 {
public:
    Paj7260U2(int gpio_line, const char* i2c_dev = "/dev/i2c-3", int addr = 0x73);
    ~Paj7260U2();

    bool init();
    void close();
    void registerCallback(Paj7260EventInterface* cb);
    bool getLastGesture(Paj7260Data& out);

private:
    bool deviceInit();
    bool readGesture(Paj7260Data& data);
    void irqLoop();

    int gpio_line_;
    const char* i2c_dev_;
    int addr_;

    int i2c_fd_;
    gpiod_chip* chip_;
    gpiod_line* line_;

    std::thread irq_thread_;
    std::atomic<bool> running_;

    std::mutex cb_mutex_;
    std::vector<Paj7260EventInterface*> callbacks_;

    Paj7260Data latest_data_;
    std::mutex data_mutex_;

    std::condition_variable cv_;
    std::mutex cv_mutex_;
    bool has_new_data_ = false;
};