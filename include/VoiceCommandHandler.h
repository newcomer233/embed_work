#pragma once
#include "SpeechCtrl.h"
#include "snake_game.h"
#include "app_controller.h"
#include "PiperSynthesizer.h"
#include "WeatherWrapper.h"
#include "AppEvent.h"
#include <sstream>
#include <map>
#include <functional>
#include <iostream>
#include "AppStateManager.h"

class VoiceCommandHandler {
public:
    using EventCallback = std::function<void(AppEvent)>;

    VoiceCommandHandler(SnakeGame& snakeRef, AppController& appRef,
                        PiperSynthesizer& synth, const std::string& apiKey,
                        const std::string& city, EventCallback cb)
        : snake(snakeRef), app(appRef), synth(synth), weather(apiKey, city),
          eventCallback(cb), speechCtrl("../model") {

        using namespace std::placeholders;

        speechCtrl.setOnUp([&]() { if (currentState == AppState::GAME) snake.setDirection(Direction::DOWN); });
        speechCtrl.setOnDown([&]() { if (currentState == AppState::GAME) snake.setDirection(Direction::UP); });
        speechCtrl.setOnLeft([&]() { if (currentState == AppState::GAME) snake.setDirection(Direction::RIGHT); });
        speechCtrl.setOnRight([&]() { if (currentState == AppState::GAME) snake.setDirection(Direction::LEFT); });

        speechCtrl.setResultCallback(std::bind(&VoiceCommandHandler::onVoiceResult, this, _1));
    }

    void setAppState(AppState state) {
        currentState = state;
    }

    void start() {
        speechCtrl.start();
    }

    void stop() {
        speechCtrl.stop();
    }

    SpeechCtrl& getSpeechCtrl() { return speechCtrl; }  // allowed main to using speech ctrl

private:
    SnakeGame& snake;
    AppController& app;
    PiperSynthesizer& synth;
    WeatherWrapper weather;
    SpeechCtrl speechCtrl;
    EventCallback eventCallback;
    AppState currentState = AppState::TIME;
    bool waitingForDigits = false;
    std::string outputPath = "../output.wav";

    std::string toLower(const std::string& input) {
        std::string result = input;
        for (char& c : result) {
            c = std::tolower(static_cast<unsigned char>(c));
        }
        return result;
    }

    void onVoiceResult(const std::string& text) {
        std::string lower = toLower(text);
    
        std::map<std::string, char> wordToDigit = {
            {"zero", '0'}, {"one", '1'}, {"two", '2'}, {"three", '3'},
            {"four", '4'}, {"five", '5'}, {"six", '6'}, {"seven", '7'},
            {"eight", '8'}, {"nine", '9'}
        };
    
        std::vector<std::string> commandKeywords = {
            "time", "game", "counter", "weather"
        };
    
        std::istringstream iss(lower);
        std::string word;
        std::vector<std::string> words;
    
        while (iss >> word) {
            words.push_back(word);
        }
    
        // ---------- waiting for 4 digital ----------
        if (waitingForDigits) {
            std::string digits;
            bool allDigits = true;
    
            for (const auto& w : words) {
                auto it = wordToDigit.find(w);
                if (it != wordToDigit.end()) {
                    digits += it->second;
                } else {
                    allDigits = false;
                    break;
                }
            }
    
            if (allDigits && digits.length() == 4) {
                std::cout << "[Voice] COUNTER: " << digits << std::endl;
                app.handleCommand(digits);
                waitingForDigits = false;
                return;
            }
    
            // judge command
            for (const auto& cmd : commandKeywords) {
                if (lower.find(cmd) != std::string::npos) {
                    std::cout << "[Voice] Exit digit input, command detected: " << cmd << std::endl;
                    waitingForDigits = false;
    
                    //deal the command
                    if (cmd == "time") eventCallback(AppEvent::EnterTime);
                    else if (cmd == "game") eventCallback(AppEvent::EnterGame);
                    else if (cmd == "counter") {
                        eventCallback(AppEvent::EnterCounter);
                        waitingForDigits = true;  
                    }
                    else if (cmd == "weather") eventCallback(AppEvent::EnterWeather);
                    return;
                }
            }
    
            // if no action
            std::cout << "[Voice] Still waiting for 4-digit input...\n";
            return;
        }
    
        // ---------- mode switch ----------
        if (lower.find("time") != std::string::npos) {
            eventCallback(AppEvent::EnterTime);
            return;
        }
    
        if (lower.find("game") != std::string::npos) {
            eventCallback(AppEvent::EnterGame);
            return;
        }
    
        if (lower.find("counter") != std::string::npos) {
            eventCallback(AppEvent::EnterCounter);
            waitingForDigits = true;
            return;
        }
    
        if (lower.find("weather") != std::string::npos) {
            eventCallback(AppEvent::EnterWeather);
            return;
        }
    }    
};