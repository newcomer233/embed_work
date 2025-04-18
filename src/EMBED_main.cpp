
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "MAX7219.h"
#include "app_controller.h"
#include "snake_game.h"
#include "SensorCtrl.h"
#include "MPU6050Ctrl.h"
#include "WeatherWrapper.h"
#include "PiperSynthesizer.h"

#include "AppStateManager.h"
#include "AppEvent.h"
#include "MPUHandler.h"
#include "GestureHandler_Event.h"
#include "VoiceCommandHandler.h"
#include "AppStateMachine.h"
    


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
    PiperSynthesizer synth(modelPath, configPath, espeakPath);

    AppStateManager stateManager;
    AppStateMachine stateMachine(stateManager);


    // Voice
    VoiceCommandHandler voiceCtrl(snake, app, synth, API_KEY, CITY, [&](AppEvent e) {
        stateMachine.handleEvent(e);
    });
    
    auto& speechCtrl = voiceCtrl.getSpeechCtrl();

    stateManager.onEnter(AppState::TIME, [&]() {
        app.handleCommand("time");
        speechCtrl.setCommandSet({"time", "game", "counter", "weather", "mode", "switch"});
        synth.synthesizeTextToFile("time mode", outputPath);
        synth.playAudioFile(outputPath);
    });
    stateManager.onExit(AppState::TIME, [&]() {
        app.shutdown();
    });
    stateManager.onEnter(AppState::GAME, [&]() {
        snake.start();
        speechCtrl.setCommandSet({"up", "down", "left", "right",
            "time", "game", "counter", "weather", "mode", "switch"});
        synth.synthesizeTextToFile("game mode", outputPath);
        synth.playAudioFile(outputPath);
    });
    stateManager.onExit(AppState::GAME, [&]() {
        snake.stop();
    });

    stateManager.onEnter(AppState::COUNTER, [&]() {
        speechCtrl.setCommandSet({"zero", "one", "two",  "three",
            "four", "five", "six", "seven",
            "eight", "nine",
        "time", "game", "counter", "weather", "mode", "switch"});
        synth.synthesizeTextToFile("counter mode", outputPath);
        synth.playAudioFile(outputPath);
        app.handleCommand("timer");
        // app.setTimerFinishedCallback( )
    });
    stateManager.onExit(AppState::COUNTER, [&]() {
        app.shutdown();
    });
    stateManager.onEnter(AppState::WEATHER, [&]() {
        app.handleCommand("temp");
        speechCtrl.setCommandSet({"time", "game", "counter", "weather", "mode", "switch"});
        synth.synthesizeTextToFile("weather mode", outputPath);
        synth.playAudioFile(outputPath);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        WeatherWrapper weather(API_KEY, CITY);
        weather.updateWeather();
        std::string cmd = weather.getWeatherCommand();
        app.handleCommand(cmd);
    });
    stateManager.onExit(AppState::WEATHER, [&]() {
        app.shutdown();
    });



    stateManager.onEnter(AppState::TIME, [&]() { voiceCtrl.setAppState(AppState::TIME); });
    stateManager.onEnter(AppState::GAME, [&]() { voiceCtrl.setAppState(AppState::GAME); });

    // MPU 
    MPU6050Ctrl mpuCtrl(13, "/dev/i2c-1", 0x68);
    MPUHandler mpuHandler([&](AppEvent e) {
        stateMachine.handleEvent(e);
    });
    mpuCtrl.setCallback(&mpuHandler);

    // Gesture
    SensorCtrl gestureCtrl(12);
    GestureHandler_Event gestureHandler(snake, stateManager);
    gestureCtrl.setGestureHandler(&gestureHandler);

    // Initialisation
    if (!gestureCtrl.init() || !mpuCtrl.init()) {
        std::cerr << "[Main] sensor init fail" << std::endl;
        return 1;
    }

    voiceCtrl.start();

    std::cout << "[Main] initial done, time mode, waiting for events..." << std::endl;
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "quit") break;
        else if (input == "time") stateMachine.handleEvent(AppEvent::EnterTime);
        else if (input == "game") stateMachine.handleEvent(AppEvent::EnterGame);
        else if (input == "counter") stateMachine.handleEvent(AppEvent::EnterCounter);
        else if (input == "weather") stateMachine.handleEvent(AppEvent::EnterWeather);
    }

    voiceCtrl.stop();
    return 0;
    return 0;
}
