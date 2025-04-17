#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <string>
#include "SpeechRecognizer.h"
    
class SpeechCtrl {
public:
    SpeechCtrl(const std::string& modelPath);
    ~SpeechCtrl();

    void start();
    void stop();

    void setOnUp(std::function<void()> cb);
    void setOnDown(std::function<void()> cb);
    void setOnLeft(std::function<void()> cb);
    void setOnRight(std::function<void()> cb);
    void setOnModeSwitch(std::function<void()> cb);
    

    void setResultCallback(std::function<void(const std::string&)> cb);
    void setCommandSet( const std::vector<std::string>& commands);
    
private:
    void listenLoop();

    std::string modelPath;
    std::thread listenThread;
    std::atomic<bool> running;

    SpeechRecognizer* recognizer;

    std::function<void()> onUp;
    std::function<void()> onDown;
    std::function<void()> onLeft;
    std::function<void()> onRight;
    std::function<void()> onModeSwitch;

    std::function<void(const std::string&)> onRawText;
    std::vector<std::string> currentCommandSet;
};
