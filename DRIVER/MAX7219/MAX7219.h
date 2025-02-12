#ifndef MAX7219_H
#define MAX7219_H

#include <cstdint>

class MAX7219 {
public:
    MAX7219();
    ~MAX7219();

    void clear_display();
    void display_pattern(const uint8_t pattern[8]);
    void set_pixel(uint8_t x, uint8_t y, bool state);
    void update_display();
    
    // 🔥 **新增功能：直接写入 MAX7219 寄存器**
    void write_register(uint8_t reg, uint8_t value);

    // **独立功能接口**
    void set_brightness(uint8_t level);  // 设置亮度（0-15）
    void set_display(bool state);        // 开启/关闭显示
    void set_decode_mode(bool enable);   // 设置 BCD 译码模式（数码管模式）
    void set_test_mode(bool enable);     // 进入/退出测试模式



private:
    int spi_fd;
    uint8_t buffer[8]; // 8x8 点阵缓存
    void initialize();
};

#endif



