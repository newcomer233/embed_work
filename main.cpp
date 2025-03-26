#include "weather.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    const string API_KEY = "fe43e68f9ac2ce5e9488824dea81a02c";
    const string CITY = "Glasgow";
    
    Weather weather(API_KEY, CITY);

    thread weatherThread([&weather]() {
        while (true) {
            weather.getCurrentWeather();
            this_thread::sleep_for(chrono::minutes(30));
        }
    });

    thread forecastThread([&weather]() {
        while (true) {
            weather.getWeatherForecast();
            this_thread::sleep_for(chrono::hours(12));
        }
    });

    weatherThread.detach();
    forecastThread.detach();

    while (true) {
        this_thread::sleep_for(chrono::hours(1));
    }

    return 0;
}