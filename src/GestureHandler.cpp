#include "GestureHandler.h"
#include <iostream>

void GestureHandler::onUp()               { std::cout << "[Gesture] 向上\n"; }
void GestureHandler::onDown()             { std::cout << "[Gesture] 向下\n"; }
void GestureHandler::onLeft()             { std::cout << "[Gesture] 向左\n"; }
void GestureHandler::onRight()            { std::cout << "[Gesture] 向右\n"; }
void GestureHandler::onForward()          { std::cout << "[Gesture] 向前推\n"; }
void GestureHandler::onBackward()         { std::cout << "[Gesture] 向后拉\n"; }
void GestureHandler::onClockwise()        { std::cout << "[Gesture] 顺时针旋转\n"; }
void GestureHandler::onCounterClockwise() { std::cout << "[Gesture] 逆时针旋转\n"; }

void GestureHandler::handleGesture(uint8_t gestureBits) {
    if (gestureBits & 0x01) onUp();
    if (gestureBits & 0x02) onDown();
    if (gestureBits & 0x04) onLeft();
    if (gestureBits & 0x08) onRight();
    if (gestureBits & 0x10) onForward();
    if (gestureBits & 0x20) onBackward();
    if (gestureBits & 0x40) onClockwise();
    if (gestureBits & 0x80) onCounterClockwise();
}
