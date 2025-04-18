#pragma once

#include "weather.h"
#include <string>

class WeatherWrapper {
public:
    WeatherWrapper(const std::string& apiKey, const std::string& city);

    // main function
    void updateWeather();                     // update and store data
    std::string getWeatherCommand() const;    // return the string for app_controller,eg."s -2"

private:
    Weather weather;
    Weather::WeatherData cachedData;

    char mapDescriptionToCode(const std::string& desc) const;
    std::string toLower(const std::string& input) const;
};