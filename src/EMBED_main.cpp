
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

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
#include <unistd.h>

//event quee
std::queue<AppEvent> eventQueue;
std::mutex queueMutex;
std::condition_variable eventCV;
bool stopEventThread = false;

void dispatchEvent(AppEvent e) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push(e);
    eventCV.notify_one();
}

void speakAsync(PiperSynthesizer& synth, const std::string& text, const std::string& outputPath) {
    std::thread([=, &synth]() {
        synth.synthesizeTextToFile(text, outputPath);
        synth.playAudioFile(outputPath);
    }).detach();
}

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

    //event handler
    std::thread eventThread([&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            eventCV.wait(lock, [&]() { return !eventQueue.empty() || stopEventThread; });

            if (stopEventThread && eventQueue.empty()) break;

            AppEvent e = eventQueue.front();
            eventQueue.pop();
            lock.unlock();

            stateMachine.handleEvent(e);
        }
    });

    // Voice
    VoiceCommandHandler voiceCtrl(snake, app, synth, stateManager,API_KEY, CITY, [&](AppEvent e) {
        dispatchEvent(e);
    });
    
    auto& speechCtrl = voiceCtrl.getSpeechCtrl();

    stateManager.onEnter(AppState::TIME, [&]() {
        app.handleCommand("time");
        speechCtrl.setCommandSet({"time", "game", "counter", "weather", "mode", "switch"});
        speakAsync(synth, "time mode", outputPath);
    });
    stateManager.onExit(AppState::TIME, [&]() {
        app.shutdown();
    });
    stateManager.onEnter(AppState::GAME, [&]() {
        snake.start();
        speechCtrl.setCommandSet({"up", "down", "left", "right",
            "time", "game", "counter", "weather", "mode", "switch"});
        speakAsync(synth, "game mode", outputPath);
    });
    stateManager.onExit(AppState::GAME, [&]() {
        snake.stop();
    });

    stateManager.onEnter(AppState::COUNTER, [&]() {
        speechCtrl.setCommandSet({"zero", "one", "two",  "three",
            "four", "five", "six", "seven",
            "eight", "nine",
        "time", "game", "counter", "weather", "mode", "switch"});
        speakAsync(synth, "counter mode", outputPath);

        app.setTimerFinishedCallback([&]() {
            speakAsync(synth, "counter finished", outputPath);
        });

        app.handleCommand("timer");

    });
    stateManager.onExit(AppState::COUNTER, [&]() {
        app.shutdown();
    });
    stateManager.onEnter(AppState::WEATHER, [&]() {
        app.handleCommand("temp");
        speechCtrl.setCommandSet({"time", "game", "counter", "weather", "mode", "switch"});
        speakAsync(synth, "weather mode", outputPath);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        WeatherWrapper weather(API_KEY, CITY);
        weather.updateWeather();
        std::string cmd = weather.getWeatherCommand();
        app.handleCommand(cmd);
    });
    stateManager.onExit(AppState::WEATHER, [&]() {
        app.shutdown();
    });


    // MPU 
    MPU6050Ctrl mpuCtrl(13, "/dev/i2c-1", 0x68);
    MPUHandler mpuHandler([&](AppEvent e) {
        dispatchEvent(e);
    });
    mpuCtrl.setCallback(&mpuHandler);

    // Gesture
    SensorCtrl gestureCtrl(12);
    GestureHandler_Event gestureHandler(snake, stateManager);
    gestureCtrl.setGestureHandler(&gestureHandler);

    // Initialisation
    if (!gestureCtrl.init() || !mpuCtrl.init()) {
        std::cerr << "[Main] sensor init fail" << std::endl;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stopEventThread = true;
        }
        eventCV.notify_one();
        eventThread.join();
        return 1;
    }

    voiceCtrl.start();
    speechCtrl.setInitialset({"time", "game", "counter", "weather", "mode", "switch"});
    std::cout << "[Main] initial done, time mode, waiting for events..." << std::endl;

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "quit") break;
        else if (input == "time") dispatchEvent(AppEvent::EnterTime);
        else if (input == "game") dispatchEvent(AppEvent::EnterGame);
        else if (input == "counter") dispatchEvent(AppEvent::EnterCounter);
        else if (input == "weather") dispatchEvent(AppEvent::EnterWeather);
    }

    voiceCtrl.stop();

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopEventThread = true;
    }
    eventCV.notify_one();
    eventThread.join();

    return 0;
}
