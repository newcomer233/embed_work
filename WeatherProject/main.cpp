#include "weather.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    // **创建线程，定期获取当前天气**
    thread weatherThread([]() {
        while (true) {
            getCurrentWeather();
            this_thread::sleep_for(chrono::minutes(30));  // 每 30 分钟更新一次
        }
    });

    // **创建线程，定期获取天气预报**
    thread forecastThread([]() {
        while (true) {
            getWeatherForecast();
            this_thread::sleep_for(chrono::hours(12));  // 每 12 小时更新一次
        }
    });

    // **让主线程保持运行**
    weatherThread.detach();
    forecastThread.detach();

    while (true) {
        this_thread::sleep_for(chrono::hours(1));
    }

    return 0;
}