#include "app_controller.h"
#include "MAX7219.h"
#include "patterns.h"

#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <cctype>
#include <limits>

AppController::AppController(MAX7219& display)
    : mode("time"),
      running(true),
      timerActive(false),
      timerPaused(false),
      timerFinished(false),
      waitingWeatherInput(false),
      awaitingTimerInput(false),
      max7219(display)
{
    startTimeMode(); // start in time mode
}

void AppController::handleCommand(const std::string& input) {
    // count the setting time
    if (awaitingTimerInput) {
        if (input == "quit") {
            shutdown();
            exit(0);
        } else if (input == "time") {
            awaitingTimerInput = false;
            startTimeMode();
        } else if (input == "temp") {
            awaitingTimerInput = false;
            waitingWeatherInput = true;
            std::cout << "weather mode ,please typing like s12 / r-5\n";
        } else if (input.size() == 4 && isdigit(input[0]) && isdigit(input[1]) &&
                   isdigit(input[2]) && isdigit(input[3])) {
            int minutes = std::stoi(input.substr(0, 2));
            int seconds = std::stoi(input.substr(2, 2));
            int totalSeconds = minutes * 60 + seconds;

            if (totalSeconds <= 0) {
                std::cout << "time must bigger than 0\n";
                return;
            }

            awaitingTimerInput = false;
            startTimerCountdown(totalSeconds);
            std::cout << "timer is on, typing p to pause or continue。\n";
        } else {
            std::cout << "error type , should be (MMSS)\n";
        }
        return;
    }

    // 模式切换
    if (input == "time") {
        startTimeMode();
    } else if (input == "timer") {
        stopAllThreads();
        max7219.displayTime(0, 0);
        awaitingTimerInput = true;
        mode = "timer";
        std::cout << "please input 4 bits number(MMSS),  0230 means 02min 30s: ";
    } else if (input == "p" && mode == "timer") {
        if (timerActive.load()) {
            timerPaused = !timerPaused;
            std::cout << (timerPaused ? "pause\n" : "continue\n");
        } else {
            std::cout << "timer is not work\n";
        }
    } else if (input == "temp") {
        waitingWeatherInput = true;
        std::cout << "input weather and temperature ( s12 / r-5)...\n";
    } else if (mode == "temp" || waitingWeatherInput) {
        processTempInput(input);
    }
}

void AppController::shutdown() {
    stopAllThreads();
}

void AppController::startTimeMode() {
    stopAllThreads();
    mode = "time";
    running = true;
    max7219.clearDisplayAll();

    blinkThread = std::thread([this]() {
        bool colonVisible = true;
        while (running.load()) {
            std::time_t now = std::time(nullptr);
            std::tm* local = std::localtime(&now);
            int hour = local->tm_hour;
            int minute = local->tm_min;

            max7219.displayTimeWithColon(hour, minute, colonVisible);
            colonVisible = !colonVisible;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::cout << "switch to time mode\n";
}

void AppController::setTimerFinishedCallback(const std::function<void()>& callback) {
    onTimerFinished = callback;
}

void AppController::startTimerCountdown(int totalSeconds) {
    stopAllThreads();
    timerActive = true;
    timerPaused = false;
    timerFinished = false;

    timerThread = std::thread([this, totalSeconds]() {
        int secondsLeft = totalSeconds;

        while (secondsLeft >= 0 && timerActive.load()) {
            if (!timerPaused.load()) {
                int min = secondsLeft / 60;
                int sec = secondsLeft % 60;
                max7219.displayTime(min, sec);

                if (secondsLeft == 0) {
                    std::cout << "timer finish!" << std::endl;
                    if (onTimerFinished) onTimerFinished();
                }

                secondsLeft--;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (timerActive) {
            timerFinished = true;
            timerBlinkThread = std::thread([this]() {
                for (int i = 0; i < 20 && timerFinished.load(); ++i) {
                    if (i % 2 == 0)
                        max7219.displayTime(0, 0);
                    else
                        max7219.clearDisplayAll();
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }

                timerFinished = false;
                max7219.displayTime(0, 0);
                awaitingTimerInput = true;
                std::cout << "please input 4 bits number(MMSS),  0230 means 02min 30s:  ";
            });
        }

        timerActive = false;
    });
}

void AppController::processTempInput(const std::string& input) {
    if (input.size() < 2 || !isalpha(input[0])) {
        std::cout << "error type , should be s12 / r-5\n";
        return;
    }

    char weather = input[0];
    std::string tempStr = input.substr(1);

    try {
        int temp = std::stoi(tempStr);
        stopAllThreads();
        mode = "temp";
        waitingWeatherInput = false;
        max7219.clearDisplayAll();

        if (weather == 's') {
            max7219.blinkWeatherWithTemp(Patterns::Sun1, Patterns::Sun2, temp);
        } else if (weather == 'r') {
            max7219.blinkWeatherWithTemp(Patterns::Rain1, Patterns::Rain2, temp);
        } else if (weather == 'h') {
            max7219.blinkWeatherWithTemp(Patterns::HeavyRain1, Patterns::HeavyRain2, temp);
        } else if (weather == 'l') {
            max7219.blinkWeatherWithTemp(Patterns::Lightning1, Patterns::Lightning2, temp);
        } else if (weather == 'c') {
            max7219.displayPattern(0, Patterns::Cloud1);
            max7219.displayTemperature(temp, 1);
        } else if (weather == 'n') {
            max7219.displayPattern(0, Patterns::Snow);
            max7219.displayTemperature(temp, 1);
        } else if (weather == 'g') {
            max7219.displayPattern(0, Patterns::G_Home);
            max7219.displayTemperature(temp, 1);
        } else {
            std::cout << "unknown weather code\n";
        }

    } catch (...) {
        std::cout << "temp type error\n";
    }
}

void AppController::stopAllThreads() {
    running = false;
    timerActive = false;
    timerPaused = false;
    timerFinished = false;

    if (blinkThread.joinable()) blinkThread.join();
    if (timerThread.joinable()) timerThread.join();
    if (timerBlinkThread.joinable()) timerBlinkThread.join();
}
