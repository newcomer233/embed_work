#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "snake.h"
#include "MAX7219.h"
#include "thread"
#include <atomic>
#include <mutex>
#include <functional> 

class SnakeGame {
public:
    SnakeGame(int width, int height, MAX7219& disp);
    ~SnakeGame();

    void start();
    void setDirection(Direction d); 
    void stop(); 


private:
    int width, height;
    int fx, fy;
    bool flash;
    int frameCount;

    Direction currentDirection; 
    Snake snake;
    MAX7219 display;
    std::thread gameThread;

    void run();
    // void setNonBlockingRawInput();
    void restoreInput();
    void generateApple();
    bool isOccupied(int x, int y) const;
    void reset();
    
    std::atomic<bool> running = true; // running status
    std::mutex directionMutex;
};

#endif
