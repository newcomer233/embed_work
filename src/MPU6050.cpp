// MPU6050.cpp
#include "MPU6050.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <cstring>
#include <sys/ioctl.h>


#define REG_PWR_MGMT_1   0x6B
#define REG_INT_ENABLE   0x38
#define REG_INT_PIN_CFG  0x37
#define REG_DATA_START   0x3B
#define REG_MOT_THR      0x1F
#define REG_MOT_DUR      0x20
#define REG_MOT_DETECT_CTRL 0x69
#define REG_ACCEL_CONFIG 0x1C
#define REG_MOTION_CTRL  0x6A

static int16_t to_int16(uint8_t hi, uint8_t lo) {
    return (int16_t)((hi << 8) | lo);
}

MPU6050::MPU6050(int gpio_line, const char* i2c_dev, int addr)
    : gpio_line_(gpio_line), i2c_dev_(i2c_dev), addr_(addr),
      i2c_fd_(-1), chip_(nullptr), line_(nullptr), running_(false) {}

MPU6050::~MPU6050() {
    close();
}

bool MPU6050::init() {
    i2c_fd_ = open(i2c_dev_, O_RDWR);
    if (i2c_fd_ < 0 || ioctl(i2c_fd_, I2C_SLAVE, addr_) < 0) {
        std::cerr << "I2C initial fail\n";
        return false;
    }

    if (!deviceInit()) {
        std::cerr << "MPU6050 initial fail\n";
        return false;
    }

    chip_ = gpiod_chip_open_by_name("gpiochip0");
    if (!chip_) return false;

    line_ = gpiod_chip_get_line(chip_, gpio_line_);
    if (!line_) return false;

    if (gpiod_line_request_falling_edge_events(line_, "mpu6050_irq") < 0) return false;

    running_ = true;
    irq_thread_ = std::thread(&MPU6050::irqLoop, this);
    return true;
}


bool MPU6050::deviceInit() {
    uint8_t cmds[][2] = {
        {REG_PWR_MGMT_1, 0x00},       // 唤醒
        {REG_INT_ENABLE, 0x40},       // 启用 Motion 中断（bit 6）
        {REG_INT_PIN_CFG, 0xB0},      // 中断引脚配置：低电平有效，推挽输出
        {REG_MOT_THR, 10},            // Motion Threshold（可调）
        {REG_MOT_DUR, 10},            // Motion Duration（可调）
        {REG_MOT_DETECT_CTRL, 0x15},  // 加速器 LPF，运动检测控制
        {REG_ACCEL_CONFIG, 0x20},     // 设置高通滤波器用于运动检测
    };

    for (auto& cmd : cmds) {
        if (write(i2c_fd_, cmd, 2) != 2) {
            std::cerr << "初始化失败: 0x" << std::hex << (int)cmd[0] << std::endl;
            return false;
        }
        usleep(1000);
    }

    uint8_t reg = 0x3A;
    if (write(i2c_fd_, &reg, 1) != 1) {
        std::cerr << "写入寄存器地址 0x3A 失败\n";
    } else {
        uint8_t tmp;
        if (read(i2c_fd_, &tmp, 1) != 1) {
            std::cerr << "读取 0x3A 寄存器失败\n";
        } else {
            std::cout << "INT_STATUS: 0x" << std::hex << (int)tmp << std::endl;
        }
    }
    

    return true;
    
}

void MPU6050::registerCallback(MPU6050EventInterface* cb) {
    std::lock_guard<std::mutex> lock(cb_mutex_);
    callbacks_.push_back(cb);
}

bool MPU6050::readRawData(MPU6050_Data& d) {
    uint8_t reg = REG_DATA_START;
    write(i2c_fd_, &reg, 1);

    uint8_t buf[14];
    if (read(i2c_fd_, buf, 14) != 14) return false;

    d.ax = to_int16(buf[0], buf[1]);
    d.ay = to_int16(buf[2], buf[3]);
    d.az = to_int16(buf[4], buf[5]);
    d.temp = to_int16(buf[6], buf[7]) / 340.0 + 36.53;
    d.gx = to_int16(buf[8], buf[9]);
    d.gy = to_int16(buf[10], buf[11]);
    d.gz = to_int16(buf[12], buf[13]);

    std::lock_guard<std::mutex> lock(data_mutex_);
    latest_ = d;
    return true;
}

bool MPU6050::getData(MPU6050_Data& out) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    out = latest_;
    return true;
}

void MPU6050::irqLoop() {
    std::cout << "[IRQ] INT monitor GPIO" << gpio_line_ << std::endl;

    while (running_) {
        const timespec ts = { 5, 0 };
        int r = gpiod_line_event_wait(line_, &ts);
        if (r == 1) {
            gpiod_line_event ev;
            if (gpiod_line_event_read(line_, &ev) == 0 &&
                ev.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {

                std::cout << "[IRQ] falling edge detected\n";

                uint8_t reg = 0x3A;
                if (write(i2c_fd_, &reg, 1) != 1) {
                    std::cerr << "fail to write 0x3A \n";
                } else {
                    uint8_t tmp;
                    if (read(i2c_fd_, &tmp, 1) != 1) {
                        std::cerr << "fail to read 0x3A \n";
                    } else {
                        std::cout << "INT_STATUS: 0x" << std::hex << (int)tmp << std::endl;
                    }
                }
                
                MPU6050_Data d;
                if (readRawData(d)) {
                    std::lock_guard<std::mutex> lock(cb_mutex_);
                    for (auto &cb : callbacks_) cb->onDataReady(d);
                }else {
                    std::cout << "[IRQ] fail to read, register may not clear\n";
                }
            }
        }
    }
}
void MPU6050::close() {
    running_ = false;
    if (irq_thread_.joinable()) irq_thread_.join();
    if (line_) gpiod_line_release(line_);
    if (chip_) gpiod_chip_close(chip_);
    if (i2c_fd_ >= 0) ::close(i2c_fd_);
}