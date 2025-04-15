#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "snake.h"
#include "max7219.h"

class SnakeGame {
public:
    SnakeGame(int width = 16, int height = 8);
    void run();
    void setDirection(Direction d); 

private:
    int width, height;
    int fx, fy;
    bool flash;
    int frameCount;
    Direction currentDirection; 

    Snake snake;
    Max7219 display;

    void setNonBlockingRawInput();
    void restoreInput();
    void generateApple();
    bool isOccupied(int x, int y) const;
};

#endif
