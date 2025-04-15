
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
#include "WeatherWrapper.h"

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

class VoiceCommandHandler {
    public:
        VoiceCommandHandler(SnakeGame& game, AppController& controller, AppState& state,
                            const std::string& apiKey, const std::string& city)
            : speechCtrl("../model"),
              snake(game),
              app(controller),
              appState(state),
              weather(apiKey, city) {
    
            // 贪吃蛇方向控制
            speechCtrl.setOnUp([this]() {
                if (appState == AppState::GAME)
                    snake.setDirection(Direction::UP);
            });
            speechCtrl.setOnDown([this]() {
                if (appState == AppState::GAME)
                    snake.setDirection(Direction::DOWN);
            });
            speechCtrl.setOnLeft([this]() {
                if (appState == AppState::GAME)
                    snake.setDirection(Direction::LEFT);
            });
            speechCtrl.setOnRight([this]() {
                if (appState == AppState::GAME)
                    snake.setDirection(Direction::RIGHT);
            });
    
            // 通用语音命令处理
            speechCtrl.setResultCallback([this](const std::string& text) {
                std::string lower = toLower(text);
    
                if (lower.find("switch") != std::string::npos || lower.find("mode") != std::string::npos) {
                    std::cout << "[Voice] 语音切换模式" << std::endl;
                    toggleMode();
                } else if(appState == AppState::TIME){                
                    if (lower.find("timer") != std::string::npos) {
                    std::cout << "[Voice] 语音进入计时器模式" << std::endl;
                    appState = AppState::TIME;
                    this->app.handleCommand("timer");
                    this->app.handleCommand("0005");
                    } else if (lower.find("weather") != std::string::npos || lower.find("check weather") != std::string::npos || lower.find("whether") != std::string::npos) {
                        std::cout << "[Voice] 语音进入天气模式" << std::endl;
                        appState = AppState::TIME;
                        this->app.handleCommand("temp");
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
                        weather.updateWeather();
                        std::string weatherCmd = weather.getWeatherCommand();
                        std::cout << "[Voice] 天气命令为: " << weatherCmd << std::endl;
                        this->app.handleCommand(weatherCmd);
                    }
                }
            });
        }
    
        void start() {
            speechCtrl.start();
        }
    
        void stop() {
            speechCtrl.stop();
        }
    
    private:
        SpeechCtrl speechCtrl;
        SnakeGame& snake;
        AppController& app;
        AppState& appState;
        WeatherWrapper weather;
    
        void toggleMode() {
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
    
        std::string toLower(const std::string& input) {
            std::string result = input;
            for (char& c : result) c = std::tolower(c);
            return result;
        }
    };
    
    


int main() {
    std::string API_KEY = "fe43e68f9ac2ce5e9488824dea81a02c";
    std::string  CITY = "Glasgow";

    MAX7219 MAX7219;
    AppController app(MAX7219);
    SnakeGame snake(16, 8, MAX7219);
    AppState currentState = AppState::TIME;

    app.setTimerFinishedCallback([]() {
        std::cout << "[Main] timer stop!" << std::endl;
    });

    // 初始化传感器
    SensorCtrl gestureCtrl(12);
    MyGestureHandler gestureHandler(snake, currentState);
    gestureCtrl.setGestureHandler(&gestureHandler);

    MPU6050Ctrl mpuCtrl(13, "/dev/i2c-1", 0x68);
    MyMPUHandler mpuHandler(app, snake, currentState);
    mpuCtrl.setCallback(&mpuHandler);

    if (!gestureCtrl.init()) {
        std::cerr << "[Main] PAJ7260 init fail" << std::endl;
        return 1;
    }

    if (!mpuCtrl.init()) {
        std::cerr << "[Main] MPU6050 init fail" << std::endl;
        return 1;
    }

 
    VoiceCommandHandler voiceCtrl(snake, app, currentState,API_KEY,CITY);
    voiceCtrl.start();

    std::cout << "[Main] initial done, time mode, waiting for INT and command..." << std::endl;
    // for debug
    std::string input;
    while (true) {
        
        std::getline(std::cin, input);

        if (input == "quit") {
            snake.stop();
            break;
        } else if (input == "state_game" && currentState != AppState::GAME) {
            currentState = AppState::GAME;
            app.shutdown();
            snake.start();
        } else if (input == "state_time" && currentState != AppState::TIME) {
            snake.stop();
            currentState = AppState::TIME;
            app.handleCommand("time");
            if(input == "temp") {
                app.handleCommand("temp");
                app.handleCommand("s12");
            }
        } else if (currentState == AppState::GAME) {
            if (input == "up") snake.setDirection(Direction::UP);
            else if (input == "down") snake.setDirection(Direction::DOWN);
            else if (input == "left") snake.setDirection(Direction::LEFT);
            else if (input == "right") snake.setDirection(Direction::RIGHT);
        }
    }

    // clear speech recognizer
    voiceCtrl.stop();
    return 0;
}
