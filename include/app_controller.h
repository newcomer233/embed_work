#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <string>
#include <atomic>
#include <thread>
#include <functional>  

class MAX7219;

class AppController {
public:
    AppController(MAX7219& display);
    void handleCommand(const std::string& input);
    void setTimerFinishedCallback(const std::function<void()>& callback); 
    void shutdown();

private:
    void startTimeMode();
    void startTimerCountdown(int totalSeconds); 
    std::function<void()> onTimerFinished;
    void processTempInput(const std::string& input);
    void stopAllThreads();

    std::string mode = "time";

    std::atomic<bool> running;
    std::atomic<bool> timerActive;
    std::atomic<bool> timerPaused;
    std::atomic<bool> timerFinished;

    bool waitingWeatherInput = false;
    bool awaitingTimerInput = false;

    std::thread blinkThread;
    std::thread timerThread;
    std::thread timerBlinkThread;

    MAX7219& max7219;
};

#endif