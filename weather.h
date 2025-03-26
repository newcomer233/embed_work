#ifndef WEATHER_H
#define WEATHER_H

#include <string>
#include <map>
#include <functional>

class Weather {
public:
    struct WeatherData {
        std::string date;
        std::string description;
        float temperature;
        int humidity;
        float windSpeed;
    };

    Weather(const std::string& api_key, const std::string& city);
    
    WeatherData getCurrentWeather();
    std::map<std::string, std::pair<float, float>> getWeatherForecast();

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
    std::string fetchWeatherData(const std::string& url);
    std::string timestampToDate(time_t timestamp) const;

    const std::string API_KEY;
    const std::string CITY;
};

#endif // WEATHER_H