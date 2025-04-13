#pragma once
#include <cstdint>

class GestureHandler {
public:
    virtual ~GestureHandler() {}

    virtual void onUp();
    virtual void onDown();
    virtual void onLeft();
    virtual void onRight();
    virtual void onForward();
    virtual void onBackward();
    virtual void onClockwise();
    virtual void onCounterClockwise();

    void handleGesture(uint8_t gestureBits);
};
