
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
#include "PiperSynthesizer.h"

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
    MyMPUHandler(AppController& controller, SnakeGame& game, AppState& state, PiperSynthesizer& synth)
        : app(controller), snake(game), appState(state), synth(synth){}

    void onMPU6050Data(const MPU6050_Data& d) override {
        std::cout << "[MPU INT] 切换模式" << std::endl;

        if (appState == AppState::TIME) {
            appState = AppState::GAME;
            app.shutdown(); 
            snake.start();
            this->synth.synthesizeTextToFile("game mode", outputPath);
            this->synth.playAudioFile(outputPath);
        } else {
            snake.stop();
            appState = AppState::TIME;
            app.handleCommand("time");
            this->synth.synthesizeTextToFile("time mode", "/home/newcomer233/output.wav");
            this->synth.playAudioFile("/home/newcomer233/output.wav");
        }
    }

private:
    AppController& app;
    SnakeGame& snake;
    AppState& appState;
    PiperSynthesizer& synth;
    
    std::string outputPath="../output.wav";
};

class VoiceCommandHandler {
    public:
        VoiceCommandHandler(SnakeGame& game, AppController& controller, AppState& state,PiperSynthesizer& synth,
                            const std::string& apiKey, const std::string& city)
            : speechCtrl("../model"),
              snake(game),
              app(controller),
              appState(state),
              weather(apiKey, city),
              synth (synth)
              {
            
            
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
                // deal with digital number
                if (waitingForDigits) {
                    // 处理 4 位数字
                    std::map<std::string, char> wordToDigit = {
                        {"zero", '0'}, {"one", '1'}, {"two", '2'}, {"three", '3'},
                        {"four", '4'}, {"five", '5'}, {"six", '6'}, {"seven", '7'},
                        {"eight", '8'}, {"nine", '9'}
                    };
                    std::istringstream iss(lower);
                    std::string word;
                    std::string digits;
                    while (iss >> word) {
                        auto it = wordToDigit.find(word);
                        if (it != wordToDigit.end()) {
                            digits += it->second;
                        }
                    }
            
                    if (digits.length() == 4) {
                        std::cout << "[Voice] 倒计时数字识别结果: " << digits << std::endl;
                        this->app.handleCommand(digits);
                        waitingForDigits = false;  // 完成输入，退出等待状态
                    } else {
                        std::cout << "[Voice] 请输入 4 位数字...\n";
                    }
            
                    return;  // 当前处理完毕，return 掉
                }
            
                if (lower.find("counter") != std::string::npos) {
                    std::cout << "[Voice] 语音进入计时器模式" << std::endl;
                    this->synth.synthesizeTextToFile("timer mode", outputPath);
                    this->synth.playAudioFile(outputPath);
                    this->app.handleCommand("timer");
            
                    waitingForDigits = true;  // 等待下一句输入数字
                    return;
                }
                
                // state mechane
                if (lower.find("switch") != std::string::npos || lower.find("mode") != std::string::npos ) {
                    std::cout << "[Voice] 语音切换模式" << std::endl;
                    toggleMode();
                    waitingForDigits = false;
                } 
                else if(appState == AppState::TIME){                
                    if (lower.find("counter") != std::string::npos) {
                        std::cout << "[Voice] 语音进入计时器模式" << std::endl;
                        this->synth.synthesizeTextToFile("timer mode", outputPath);
                        this->synth.playAudioFile(outputPath);
                        this->app.handleCommand("timer");
                        waitingForDigits = true;
                    } 
                    else if (lower.find("weather") != std::string::npos) {
                        std::cout << "[Voice] 语音进入天气模式" << std::endl;
                        waitingForDigits = false;
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
            speechCtrl.setCommandSet({"counter", "weather", "mode"});
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
        PiperSynthesizer& synth;
        
        std::string outputPath="../output.wav";
        bool waitingForDigits = false;

        void toggleMode() {
            if (appState == AppState::TIME) {
                appState = AppState::GAME;
                speechCtrl.setCommandSet({"up", "down", "left", "right","mode"});
                this->synth.synthesizeTextToFile("game mode", outputPath);
                this->synth.playAudioFile(outputPath);
                app.shutdown();
                snake.start();
            } else {
                appState = AppState::TIME;
                speechCtrl.setCommandSet({"counter", "weather","mode",
                    "zero", "one", "two",  "three",
                    "four", "five", "six", "seven",
                    "eight", "nine"});
                this->synth.synthesizeTextToFile("time mode", outputPath);
                this->synth.playAudioFile(outputPath);
                snake.stop();
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
    std::string modelPath = "../piper_models/en_US-amy-medium.onnx";
    std::string configPath = modelPath + ".json";
    std::string espeakPath = "/usr/lib/aarch64-linux-gnu/espeak-ng-data";
    std::string outputPath = "../output.wav";

    std::string API_KEY = "fe43e68f9ac2ce5e9488824dea81a02c";
    std::string  CITY = "Glasgow";

    MAX7219 MAX7219;
    AppController app(MAX7219);
    SnakeGame snake(16, 8, MAX7219);
    AppState currentState = AppState::TIME;
    PiperSynthesizer synth(modelPath, configPath, espeakPath);

    app.setTimerFinishedCallback([]() {
        std::cout << "[Main] timer stop!" << std::endl;
    });

    // 初始化传感器
    SensorCtrl gestureCtrl(12);
    MyGestureHandler gestureHandler(snake, currentState);
    gestureCtrl.setGestureHandler(&gestureHandler);

    MPU6050Ctrl mpuCtrl(13, "/dev/i2c-1", 0x68);
    MyMPUHandler mpuHandler(app, snake, currentState,synth);
    mpuCtrl.setCallback(&mpuHandler);

    if (!gestureCtrl.init()) {
        std::cerr << "[Main] PAJ7260 init fail" << std::endl;
        return 1;
    }

    if (!mpuCtrl.init()) {
        std::cerr << "[Main] MPU6050 init fail" << std::endl;
        return 1;
    }

 
    VoiceCommandHandler voiceCtrl(snake, app, currentState,synth,API_KEY,CITY);
    voiceCtrl.start();

    std::cout << "[Main] initial done, time mode, waiting for INT and command..." << std::endl;
    // for debug
    std::string input;
    uint8_t test_state = true;
    while (test_state) {
        
        std::getline(std::cin, input);
        if (input == "quit"){
            test_state=false;
        } else {
            test_state=true;
        }
        // if (input == "quit") {
        //     snake.stop();
        //     break;
        // } else if (input == "state_game" && currentState != AppState::GAME) {
        //     currentState = AppState::GAME;
        //     app.shutdown();
        //     snake.start();
        // } else if (input == "state_time" && currentState != AppState::TIME) {
        //     snake.stop();
        //     currentState = AppState::TIME;
        //     app.handleCommand("time");
        //     if(input == "temp") {
        //         app.handleCommand("temp");
        //         app.handleCommand("s12");
        //     }
        // } else if (currentState == AppState::GAME) {
        //     if (input == "up") snake.setDirection(Direction::UP);
        //     else if (input == "down") snake.setDirection(Direction::DOWN);
        //     else if (input == "left") snake.setDirection(Direction::LEFT);
        //     else if (input == "right") snake.setDirection(Direction::RIGHT);
        // }
    }

    // clear speech recognizer
    voiceCtrl.stop();
    return 0;
}
