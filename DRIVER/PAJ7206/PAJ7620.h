#ifndef PAJ7620_H
#define PAJ7620_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>

class PAJ7620 {
public:
    PAJ7620(const std::string& i2c_device = "/dev/i2c-3");
    ~PAJ7620();

    bool initialize();
    std::string getGesture();

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> dataReady{false};

private:
    int i2c_fd;
    std::string i2c_device_path;

    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t &value);
};

#endif
