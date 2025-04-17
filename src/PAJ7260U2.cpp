#include "PAJ7260U2.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <algorithm>
#include <sys/ioctl.h>

#define GES_RESULT_REG 0x43
#define EVENT_TYPE_REG  0x44
#define INIT_BANK_REG  0xEF

Paj7260U2::Paj7260U2(int gpio_line, const char* i2c_dev, int addr)
    : gpio_line_(gpio_line), i2c_dev_(i2c_dev), addr_(addr),
      i2c_fd_(-1), chip_(nullptr), line_(nullptr), running_(false) {}

Paj7260U2::~Paj7260U2() {
    close();
}

bool Paj7260U2::init() {
    i2c_fd_ = open(i2c_dev_, O_RDWR);
    if (i2c_fd_ < 0 || ioctl(i2c_fd_, I2C_SLAVE, addr_) < 0) {
        std::cerr << "I2C initial fail\n";
        return false;
    }

    if (!deviceInit()) {
        std::cerr << "PAJ7260U2 initial fail\n";
        return false;
    }

    chip_ = gpiod_chip_open_by_name("gpiochip0");
    if (!chip_) return false;

    line_ = gpiod_chip_get_line(chip_, gpio_line_);
    if (!line_) return false;

    if (gpiod_line_request_falling_edge_events(line_, "paj7260_irq") < 0) return false;

    running_ = true;
    irq_thread_ = std::thread(&Paj7260U2::irqLoop, this);
    return true;
}

bool Paj7260U2::deviceInit() {
    struct RegPair {
        uint8_t reg;
        uint8_t val;
    };

    const RegPair init_regs[] = {
        {0xEF, 0x00},
        {0x41, 0xFF}, // Enable all gesture interrupts
        {0x42, 0x01}, // Enable interrupt trigger
        {0x46, 0x2D}, {0x47, 0x0F}, {0x48, 0x80},
        {0x49, 0x00}, {0x4A, 0x40}, {0x4B, 0x00}, {0x4C, 0x20}, {0x4D, 0x00},
        {0x51, 0x10}, {0x5C, 0x02}, {0x5E, 0x10},
        {0x80, 0x41}, {0x81, 0x44}, {0x82, 0x0C}, {0x83, 0x20}, {0x84, 0x20},
        {0x85, 0x00}, {0x86, 0x10}, {0x87, 0x00}, {0x8B, 0x01}, {0x8D, 0x00},
        {0x90, 0x0C}, {0x91, 0x0C}, {0x93, 0x0D}, {0x94, 0x0A}, {0x95, 0x0A},
        {0x96, 0x0C}, {0x97, 0x05}, {0x9A, 0x14}, {0x9C, 0x3F}, {0x9F, 0xF9},
        {0xA0, 0x48}, {0xA5, 0x19}
    };

    for (const auto& reg : init_regs) {
        uint8_t buf[2] = {reg.reg, reg.val};
        if (write(i2c_fd_, buf, 2) != 2) {
            std::cerr << "fail to initial: 0x" << std::hex << int(reg.reg) << std::endl;
            return false;
        }
        usleep(1000); // delay for stable
    }

    // switch to bank1 : start
    uint8_t bank1_enable[] = {0xEF, 0x01};
    write(i2c_fd_, bank1_enable, 2);

    uint8_t gesture_start[] = {0x72, 0x01}; // gesture engine
    write(i2c_fd_, gesture_start, 2);

    // switch to bank0
    uint8_t bank0_back[] = {0xEF, 0x00};
    write(i2c_fd_, bank0_back, 2);

    uint8_t reg;

    // in case some time stuck in low
    reg = GES_RESULT_REG;
    if (write(i2c_fd_, &reg, 1) == 1) {
        uint8_t tmp;
        read(i2c_fd_, &tmp, 1);
    }

    reg = EVENT_TYPE_REG;
    if (write(i2c_fd_, &reg, 1) == 1) {
        uint8_t tmp;
        read(i2c_fd_, &tmp, 1);
    }



    return true;
}

void Paj7260U2::registerCallback(Paj7260EventInterface* cb) {
    std::lock_guard<std::mutex> lock(cb_mutex_);
    callbacks_.push_back(cb);
}

bool Paj7260U2::readGesture(Paj7260Data& data) {
    uint8_t reg = GES_RESULT_REG;
    if (write(i2c_fd_, &reg, 1) != 1) return false;
    if (read(i2c_fd_, &data.gesture, 1) != 1) return false;

    reg = EVENT_TYPE_REG;
    if (write(i2c_fd_, &reg, 1) != 1) return false;
    if (read(i2c_fd_, &data.proximity, 1) != 1) return false;

    // // 🆕 修复：强制清除中断标志
    // uint8_t clear[] = {GES_RESULT_REG, 0x00};
    // write(i2c_fd_, clear, 2);

    // // 🆕 可选：重新启用所有中断（保险操作）
    // uint8_t enable[] = {0x41, 0xFF};
    // write(i2c_fd_, enable, 2);

    return true;
}


bool Paj7260U2::getLastGesture(Paj7260Data& out) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    out = latest_data_;
    return true;
}

void Paj7260U2::irqLoop() {
    std::cout << "[IRQ] 中断线程已启动，监听 GPIO" << gpio_line_ << std::endl;

    while (running_) {
        const timespec ts = { 5, 0 };
        int r = gpiod_line_event_wait(line_, &ts);
        if (r == 1) {
            gpiod_line_event ev;
            if (gpiod_line_event_read(line_, &ev) == 0 &&
                ev.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {

                std::cout << "[IRQ] 检测到下降沿中断\n";

                Paj7260Data d;
                if (readGesture(d)) {
                    {
                        std::lock_guard<std::mutex> data_lock(data_mutex_);
                        latest_data_ = d;
                    }
                    {
                        std::lock_guard<std::mutex> flag_lock(cv_mutex_);
                        has_new_data_ = true;
                    }
                    cv_.notify_all();

                    std::lock_guard<std::mutex> lock(cb_mutex_);
                    for (auto& cb : callbacks_) {
                        cb->onGestureDetected(d);
                    }
                } else {
                    std::cout << "[IRQ] 读取失败，可能中断未正确清除\n";
                }
            }
        }
    }
}

void Paj7260U2::close() {
    running_ = false;
    if (irq_thread_.joinable()) irq_thread_.join();
    if (line_) gpiod_line_release(line_);
    if (chip_) gpiod_chip_close(chip_);
    if (i2c_fd_ >= 0) ::close(i2c_fd_);
}
