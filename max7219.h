#ifndef MAX7219_H
#define MAX7219_H

#include <cstdint>

class Max7219 {
public:
    Max7219(int numDevices);
    ~Max7219();
    void init();
    void clear();
    void setPixel(int x, int y, bool on);
    void refresh();
    bool flipX = false;
    bool flipY = true;
    

private:
    int numDevices;
    int spi_fd;
    uint8_t buffer[8][2]; 
    void sendData(int device, uint8_t reg, uint8_t data);
    void sendAll(uint8_t reg, uint8_t data);
};

#endif
