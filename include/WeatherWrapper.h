#pragma once

#include "weather.h"
#include <string>

class WeatherWrapper {
public:
    WeatherWrapper(const std::string& apiKey, const std::string& city);

    // 主功能
    void updateWeather();                     // 更新并缓存当前天气数据
    std::string getWeatherCommand() const;    // 返回类似 "r-2" 的命令字符串

private:
    Weather weather;
    Weather::WeatherData cachedData;

    char mapDescriptionToCode(const std::string& desc) const;
    std::string toLower(const std::string& input) const;
};