#ifndef WEATHER_H
#define WEATHER_H

#include <string>
#include <map>

// 结构体：存储当前天气信息
struct WeatherData {
    std::string date;        // 日期
    std::string description; // 天气状况
    float temperature;       // 温度（°C）
    int humidity;            // 湿度（%）
    float windSpeed;         // 风速（m/s）
};

// **获取当前天气**
WeatherData getCurrentWeather();

// **获取未来3天天气预报**
std::map<std::string, std::pair<float, float>> getWeatherForecast();

#endif // WEATHER_H