#include "snake_game.h"
#include <thread>
#include <chrono>

int main() {
    SnakeGame game;

    // Use thread to simulate external input
    std::thread control([&]() {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2s);
        game.setDirection(DOWN);
        std::this_thread::sleep_for(2s);
        game.setDirection(LEFT);
        std::this_thread::sleep_for(2s);
        game.setDirection(UP);
        std::this_thread::sleep_for(2s);
        game.setDirection(RIGHT);
    });

    game.run();

    if (control.joinable())
        control.join();

    return 0;
}
