#include "weather.h"
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <ctime>
#include <iomanip>

using namespace std;

Weather::Weather(const string& api_key, const string& city) 
    : API_KEY(api_key), CITY(city) {}

size_t Weather::WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

string Weather::fetchWeatherData(const string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Weather::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return response;
}

string Weather::timestampToDate(time_t timestamp) const {
    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return string(buffer);
}

Weather::WeatherData Weather::getCurrentWeather() {
    string url = "https://api.openweathermap.org/data/2.5/weather?q=" + CITY +
                "&appid=" + API_KEY + "&units=metric";

    string weatherData = fetchWeatherData(url);
    if (weatherData.empty()) return {"", "", 0.0, 0, 0.0};

    Json::Reader reader;
    Json::Value jsonWeather;
    if (!reader.parse(weatherData, jsonWeather)) return {"", "", 0.0, 0, 0.0};

    WeatherData data;
    data.date = timestampToDate(jsonWeather["dt"].asLargestInt());
    data.description = jsonWeather["weather"][0]["description"].asString();
    data.temperature = jsonWeather["main"]["temp"].asFloat();
    data.humidity = jsonWeather["main"]["humidity"].asInt();
    data.windSpeed = jsonWeather["wind"]["speed"].asFloat();

    cout << "Current Weather:" << endl
         << "Date: " << data.date << endl
         << "Weather: " << data.description << endl
         << "Temperature: " << data.temperature << "°C" << endl
         << "Humidity: " << data.humidity << "%" << endl
         << "Wind Speed: " << data.windSpeed << " m/s" << endl
         << "--------------------------------------" << endl;

    return data;
}

map<string, pair<float, float>> Weather::getWeatherForecast() {
    string url = "https://api.openweathermap.org/data/2.5/forecast?q=" + CITY +
                "&appid=" + API_KEY + "&units=metric";

    string forecastData = fetchWeatherData(url);
    if (forecastData.empty()) return {};

    Json::Reader reader;
    Json::Value jsonForecast;
    if (!reader.parse(forecastData, jsonForecast)) return {};

    string today = timestampToDate(time(nullptr));
    map<string, pair<float, float>> dailyTemps;

    for (const auto& item : jsonForecast["list"]) {
        string date = timestampToDate(item["dt"].asLargestInt());
        if (date <= today) continue;
        if (dailyTemps.size() >= 3) break;

        float temp = item["main"]["temp"].asFloat();

        if (!dailyTemps.count(date)) {
            dailyTemps[date] = {temp, temp};
        } else {
            dailyTemps[date].first = min(dailyTemps[date].first, temp);
            dailyTemps[date].second = max(dailyTemps[date].second, temp);
        }
    }

    cout << "3-Day Weather Forecast:" << endl;
    for (const auto& [date, temps] : dailyTemps) {
        cout << "Date: " << date
             << "  Temperature: " << temps.first << "°C ~ " << temps.second << "°C" << endl;
    }
    cout << "--------------------------------------" << endl;

    return dailyTemps;
}