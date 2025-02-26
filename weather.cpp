#include "weather.h"
#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <map>
#include <ctime>
#include <iomanip>

using namespace std;

const string API_KEY = "fe43e68f9ac2ce5e9488824dea81a02c";  // OpenWeather API Key
const string CITY = "Glasgow";  // 查询天气的城市

// **HTTP 请求回调函数**（用于接收 API 响应数据）
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// **发送 HTTP 请求并获取 API 数据**
string fetchWeatherData(const string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return response;
}

// **时间戳转换为 YYYY-MM-DD 格式**
string timestampToDate(time_t timestamp) {
    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return string(buffer);
}

// **获取当前天气**
/*
   选择 Weather API 的原因：
   1. Weather API 提供的是 "实时天气数据"，可以获取当前温度、湿度、风速等信息。
   2. Forecast API 主要用于未来天气预测，无法提供 "实时温度"。

   为什么返回 WeatherData 结构体？
   - 这样可以方便其他代码调用，而不仅仅是输出到终端。
*/
WeatherData getCurrentWeather() {
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

    cout << "Current Weather:" << endl;
    cout << "Date: " << data.date << endl;
    cout << "Weather: " << data.description << endl;
    cout << "Temperature: " << data.temperature << "°C" << endl;
    cout << "Humidity: " << data.humidity << "%" << endl;
    cout << "Wind Speed: " << data.windSpeed << " m/s" << endl;
    cout << "--------------------------------------" << endl;

    return data;
}

// **获取未来 3 天的天气预报**
/*
   选择 Forecast API 的原因：
   1. 提供未来 5 天的天气，每天 8 个数据点（每 3 小时更新一次）。
   2. 通过遍历数据，可以计算 "最高温度" 和 "最低温度"。

   计算 min/max 温度的方法：
   1. 遍历 API 返回的 `list`（未来 5 天，每 3 小时的数据）。
   2. 统计每天的 8 个数据点，计算最高温度和最低温度。
   3. 只显示未来 3 天的数据，不包括今天。
*/
map<string, pair<float, float>> getWeatherForecast() {
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

        if (dailyTemps.find(date) == dailyTemps.end()) {
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