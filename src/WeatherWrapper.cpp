#include "WeatherWrapper.h"
#include <cmath>
#include <algorithm>

WeatherWrapper::WeatherWrapper(const std::string& apiKey, const std::string& city)
    : weather(apiKey, city) {}

void WeatherWrapper::updateWeather() {
    cachedData = weather.getCurrentWeather();
}

std::string WeatherWrapper::getWeatherCommand() const {
    char code = mapDescriptionToCode(cachedData.description);
    int roundedTemp = static_cast<int>(std::round(cachedData.temperature));
    return std::string(1, code) + std::to_string(roundedTemp);  // e.g., "r-2"
}

char WeatherWrapper::mapDescriptionToCode(const std::string& desc) const {
    std::string lower = toLower(desc);

    if (lower.find("rain") != std::string::npos)
        return 'r';
    if (lower.find("snow") != std::string::npos)
        return 'n';
    if (lower.find("cloud") != std::string::npos)
        return 'c';
    if (lower.find("storm") != std::string::npos || lower.find("thunder") != std::string::npos)
        return 'l';
    if (lower.find("sun") != std::string::npos || lower.find("clear") != std::string::npos)
        return 's';

    return 'g';  // original 
}

std::string WeatherWrapper::toLower(const std::string& input) const {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
