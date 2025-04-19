#include "SpeechCtrl.h"
#include "keywords.h" 
#include <iostream>
#include <mutex>
#include <regex>

SpeechCtrl::SpeechCtrl(const std::string& modelPath)
    : modelPath(modelPath), running(false), recognizer(nullptr) {}

SpeechCtrl::~SpeechCtrl() {
    stop();
}

void SpeechCtrl::start() {
    if (running) return;
    running = true;

    recognizer = new SpeechRecognizer(modelPath);
    recognizer->setCallback([this](const std::string& resultJson) {
        std::cout << "[SpeechCtrl] Raw result: " << resultJson << std::endl;

        auto textStart = resultJson.find("text");
        if (textStart != std::string::npos) {
            auto colonPos = resultJson.find(":", textStart);
            if (colonPos != std::string::npos) {
                auto quoteStart = resultJson.find("\"", colonPos + 1);
                auto quoteEnd = resultJson.find("\"", quoteStart + 1);
                if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                    std::string text = resultJson.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                
                std::cout << "[SpeechCtrl] Parsed Text: " << text << std::endl;

                // RAW DATA
                if (onRawText) onRawText(text);

                    // MATCHING ACTION
                    for (const auto& [key, action] : keywordMap) {
                        if (text.find(key) != std::string::npos) {
                            if (action == "UP" && onUp) onUp();
                            else if (action == "DOWN" && onDown) onDown();
                            else if (action == "LEFT" && onLeft) onLeft();
                            else if (action == "RIGHT" && onRight) onRight();
                            else if (action == "MODE" && onModeSwitch) onModeSwitch();
                            break;
                        }
                    }
                }
            }
        }
    });
    listenThread = std::thread([this]() {
        recognizer->start();
        while (running);
        recognizer->stop();
    });
}

void SpeechCtrl::stop() {
    if (!running) return;
    running = false;
    if (listenThread.joinable()) listenThread.join();
    delete recognizer;
    recognizer = nullptr;
}

void SpeechCtrl::setOnUp(std::function<void()> cb) { onUp = cb; }
void SpeechCtrl::setOnDown(std::function<void()> cb) { onDown = cb; }
void SpeechCtrl::setOnLeft(std::function<void()> cb) { onLeft = cb; }
void SpeechCtrl::setOnRight(std::function<void()> cb) { onRight = cb; }
void SpeechCtrl::setOnModeSwitch(std::function<void()> cb) { onModeSwitch = cb; }
void SpeechCtrl::setResultCallback(std::function<void(const std::string&)> cb) {
    onRawText = cb;
}

void SpeechCtrl::setCommandSet(const std::vector<std::string>& cmdList) {
    std::lock_guard<std::mutex> lock(commandMutex);
    std::vector<std::string> updatedCmdList = cmdList;

    // automatically add [unk]
    if (std::find(updatedCmdList.begin(), updatedCmdList.end(), "[unk]") == updatedCmdList.end()) {
        updatedCmdList.push_back("[unk]");
    }

    if (updatedCmdList == currentCommandSet) {
        std::cout << "[SpeechCtrl] Command set unchanged, skipping update." << std::endl;
        return;
    }

    currentCommandSet = updatedCmdList;

    if (recognizer) {
        recognizer->setGrammar(currentCommandSet);
    } else {
        std::cout << "[SpeechCtrl] recognizer not started yet, command set will be applied on start()." << std::endl;
    }
}
