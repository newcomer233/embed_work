#include "MAX7219.h"
#include <iostream>
#include <unistd.h>

int main() {
    MAX7219 max7219;

    // 自定义 8x8 LED 显示图案（心形）
    uint8_t pattern[8] = {
    0b00011000, // 第 1 行（最上面）
    0b00111100, // 第 2 行
    0b01111110, // 第 3 行
    0b11111111, // 第 4 行
    0b01111110, // 第 5 行
    0b00111100, // 第 6 行
    0b00011000, // 第 7 行
    0b00000000  // 第 8 行（最下面）
    };

    // 🔥 **测试：手动调整亮度**
    std::cout << "🔆 逐步调整亮度..." << std::endl;
    for (int i = 0; i <= 15; i++) {
        max7219.set_brightness(i);
        std::cout << "亮度设置为：" << i << std::endl;
        sleep(1);
    };

    // 🔥 **测试：关闭 & 开启显示**
    std::cout << "🔴 关闭显示 2 秒..." << std::endl;
    max7219.set_display(false);
    sleep(2);
    
    std::cout << "🟢 重新开启显示！" << std::endl;
    max7219.set_display(true);
    sleep(2);

    // 🔥 **测试：打开 & 关闭测试模式**
    std::cout << "🔬 启动测试模式（全部点亮）..." << std::endl;
    max7219.set_test_mode(true);
    sleep(2);
    
    std::cout << "🛑 关闭测试模式！" << std::endl;
    max7219.set_test_mode(false);   


    std::cout << "✅ 显示 8x8 LED 图案！" << std::endl;
    max7219.display_pattern(pattern);

    return 0;
}
