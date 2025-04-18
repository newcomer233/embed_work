#include "snake_game.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>


SnakeGame::SnakeGame(int width, int height, MAX7219& disp)
    : width(width),
      height(height),

      running(false),
      fx(0),
      fy(0),
      flash(true),
      frameCount(0),
      currentDirection(RIGHT),
      snake(width, height),
      display(disp) {}

SnakeGame::~SnakeGame() {
    stop(); // ensure no threads remain
}

// void SnakeGame::setNonBlockingRawInput() {
//     struct termios newt;
//     tcgetattr(STDIN_FILENO, &newt);
//     newt.c_lflag &= ~(ICANON | ECHO);
//     newt.c_cc[VMIN] = 0;
//     newt.c_cc[VTIME] = 0;
//     tcsetattr(STDIN_FILENO, TCSANOW, &newt);

//     int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
//     fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
// }

void SnakeGame::restoreInput() {
    struct termios default_t;
    tcgetattr(STDIN_FILENO, &default_t);
    default_t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &default_t);
}

bool SnakeGame::isOccupied(int x, int y) const {
    for (const auto& p : snake.getBody())
        if (p.second == x && p.first == y)
            return true;
    return false;
}

void SnakeGame::generateApple() {
    do {
        fx = rand() % width;
        fy = rand() % height;
    } while (isOccupied(fx, fy));
}

void SnakeGame::setDirection(Direction d) {
    std::lock_guard<std::mutex> lock(directionMutex);
    // don't move to ass
    if ((currentDirection == UP && d == DOWN) ||
        (currentDirection == DOWN && d == UP) ||
        (currentDirection == LEFT && d == RIGHT) ||
        (currentDirection == RIGHT && d == LEFT)) {
        return;
    }
    currentDirection = d;
}
void SnakeGame::start(){
    if(running) return;
    running = true;
    gameThread = std::thread(&SnakeGame::run, this);
}

void SnakeGame::run() {
    // bool running = true;

    // setNonBlockingRawInput();
    srand(time(0));
    generateApple();

    while (running) {
        // char buf[3];
        // int n = read(STDIN_FILENO, buf, sizeof(buf));
        // if (n == 3 && buf[0] == '\033' && buf[1] == '[') {
        //     switch (buf[2]) {
        //         case 'A': setDirection(UP); break;
        //         case 'B': setDirection(DOWN); break;
        //         case 'C': setDirection(RIGHT); break;
        //         case 'D': setDirection(LEFT); break;
        //     }
        // } else if (n == 1 && buf[0] == 'q') {
        //     break;
        // }
        // if (n == 1 && buf[0] == 'q') {
        //         break;
        // }
        Direction dir;
        {
            std::lock_guard<std::mutex> lock(directionMutex);
            dir = currentDirection;
        }
        if (!running) break; 


        if (frameCount % 2 == 0) {
            snake.setDirection(dir);
            snake.move();
            if (snake.checkCollision()) break;
            if (snake.eats(fx, fy)) {
                snake.grow();
                generateApple();
            }
        }

        flash = !flash;
        display.clear();
        for (auto &p : snake.getBody())
            display.setPixel(p.second, p.first, true);
        if (flash)
            display.setPixel(fx, fy, true);
        display.refresh();

        frameCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(125));
    }

    for (int i = 0; i < 3; ++i) {
        display.clear();
        for (auto &p : snake.getBody())
            display.setPixel(p.second, p.first, true);
        display.setPixel(fx, fy, true);
        display.refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        display.clear();
        display.refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    display.clear();
    display.refresh();
    restoreInput();
    std::cout << "Game Over" << std::endl;
}

void SnakeGame::stop() {
    running = false;
    if(gameThread.joinable()){
        gameThread.join();
    }
}

