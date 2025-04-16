#ifndef MAX7219_H
#define MAX7219_H

#include <cstdint>
#include "patterns.h"

class MAX7219 {
public:
    MAX7219();
    ~MAX7219();
    //common functions don't touch
    void initialize();
    void clearDisplay();
    void clearDisplayAll();
    void setBrightness(uint8_t brightness);

    // for weather display
    void displayPattern(int device, const uint8_t pattern[8]);
    void displayTwoDigits(int number, int device);
    void displayTime(int hour, int minute);
    void displayTemperature(int temp, int device);
    void displayTimeWithColon(int hour, int minute, bool colonVisible);
    void blinkWeatherWithTemp(const uint8_t pattern1[8], const uint8_t pattern2[8], int temp, int duration_ms = 3000, int interval_ms = 500);
    void writeRegister(uint8_t reg0, uint8_t value0, uint8_t reg1, uint8_t value1);

    // for snake game
    void setPixel(int x, int y, bool on);
    void refresh();
    bool flipX = false;
    bool flipY = true;
    void clear (){ clearDisplayAll(); }
private:
    int spi_fd;
    uint8_t devicePatterns[2][8];
    void sendData(int device, uint8_t reg, uint8_t value);

    //for snake game6
    int numDevices;
    uint8_t buffer[8][2]; // 每个 device 一列（最多16列）
    void sendAll(uint8_t reg, uint8_t data);

};

#endif
