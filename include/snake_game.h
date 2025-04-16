#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "snake.h"
#include "MAX7219.h"
#include "thread"
#include <atomic>

class SnakeGame {
public:
    SnakeGame(int width, int height, MAX7219& disp);
    ~SnakeGame();
    // void run();
    void start();
    void setDirection(Direction d); // 外部方向接口
    void stop(); // 停止游戏

private:
    int width, height;
    int fx, fy;
    bool flash;
    int frameCount;

    Direction currentDirection; // 当前方向
    Snake snake;
    MAX7219 display;
    std::thread gameThread;

    void run();
    // void setNonBlockingRawInput();
    void restoreInput();
    void generateApple();
    bool isOccupied(int x, int y) const;

    std::atomic<bool> running = true; // 游戏运行状态
};

#endif
