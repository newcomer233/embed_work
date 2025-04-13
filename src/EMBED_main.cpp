
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "MAX7219.h"
#include "app_controller.h"
#include "snake_game.h"
#include "SensorCtrl.h"
#include "MPU6050Ctrl.h"
#include "SpeechCtrl.h"


enum class AppState { TIME, GAME };

class MyGestureHandler : public GestureHandler {
public:
    MyGestureHandler(SnakeGame& game, AppState& state) : snake(game), appState(state) {}

    void onUp() override {
        if (appState == AppState::GAME) snake.setDirection(Direction::UP);
    }

    void onDown() override {
        if (appState == AppState::GAME) snake.setDirection(Direction::DOWN);
    }

    void onLeft() override {
        if (appState == AppState::GAME) snake.setDirection(Direction::LEFT);
    }

    void onRight() override {
        if (appState == AppState::GAME) snake.setDirection(Direction::RIGHT);
    }

private:
    SnakeGame& snake;
    AppState& appState;
};

class MyMPUHandler : public MPU6050DataCallback {
public:
    MyMPUHandler(AppController& controller, SnakeGame& game, AppState& state)
        : app(controller), snake(game), appState(state) {}

    void onMPU6050Data(const MPU6050_Data& d) override {
        std::cout << "[MPU INT] 切换模式" << std::endl;

        if (appState == AppState::TIME) {
            appState = AppState::GAME;
            app.shutdown(); 
            snake.start();
        } else {
            snake.stop();
            appState = AppState::TIME;
            app.handleCommand("time");
        }
    }

private:
    AppController& app;
    SnakeGame& snake;
    AppState& appState;
};

int main() {
    MAX7219 MAX7219;
    AppController controller(MAX7219);
    SnakeGame snake(16, 8, MAX7219);
    AppState currentState = AppState::TIME;

    controller.setTimerFinishedCallback([]() {
        std::cout << "[Main] timer stop!" << std::endl;
    });

    // 初始化传感器
    SensorCtrl gestureCtrl(12);
    MyGestureHandler gestureHandler(snake, currentState);
    gestureCtrl.setGestureHandler(&gestureHandler);

    MPU6050Ctrl mpuCtrl(13, "/dev/i2c-1", 0x68);
    MyMPUHandler mpuHandler(controller, snake, currentState);
    mpuCtrl.setCallback(&mpuHandler);

    if (!gestureCtrl.init()) {
        std::cerr << "[Main] PAJ7260 init fail" << std::endl;
        return 1;
    }

    if (!mpuCtrl.init()) {
        std::cerr << "[Main] MPU6050 init fail" << std::endl;
        return 1;
    }
    // === 初始化语音识别控制 ===
    SpeechCtrl speechCtrl("../model");
    speechCtrl.setOnUp([&]() {
        if (currentState == AppState::GAME) snake.setDirection(Direction::UP);
    });
    speechCtrl.setOnDown([&]() {
        if (currentState == AppState::GAME) snake.setDirection(Direction::DOWN);
    });
    speechCtrl.setOnLeft([&]() {
        if (currentState == AppState::GAME) snake.setDirection(Direction::LEFT);
    });
    speechCtrl.setOnRight([&]() {
        if (currentState == AppState::GAME) snake.setDirection(Direction::RIGHT);
    });
    speechCtrl.start(); 

    std::cout << "[Main] initial done, time mode, waiting for INT and command..." << std::endl;

    std::string input;
    while (true) {
        std::getline(std::cin, input);

        if (input == "quit") {
            snake.stop();
            break;
        } else if (input == "state_game" && currentState != AppState::GAME) {
            currentState = AppState::GAME;
            controller.handleCommand("game");
            snake.start();
        } else if (input == "state_time" && currentState != AppState::TIME) {
            snake.stop();
            currentState = AppState::TIME;
            controller.handleCommand("time");
        } else if (currentState == AppState::GAME) {
            if (input == "up") snake.setDirection(Direction::UP);
            else if (input == "down") snake.setDirection(Direction::DOWN);
            else if (input == "left") snake.setDirection(Direction::LEFT);
            else if (input == "right") snake.setDirection(Direction::RIGHT);
        }
    }

    // clear speech recognizer
    speechCtrl.stop();
    return 0;
}
