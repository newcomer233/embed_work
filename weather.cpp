#include <iostream>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <map>
#include <ctime>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

const string API_KEY = "fe43e68f9ac2ce5e9488824dea81a02c";  // OpenWeather API Key
const string CITY = "Glasgow";  // 查询天气的城市

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
   2. Forecast API 主要用于未来天气预测，而不能提供精确的 "实时温度"。

   线程管理：
   - 该函数运行在独立的线程中，每 30 分钟自动获取一次最新天气。
   - 使用 `this_thread::sleep_for()` 休眠 30 分钟，不阻塞主线程。
*/
void getCurrentWeather() {
    string url = "https://api.openweathermap.org/data/2.5/weather?q=" + CITY +
                 "&appid=" + API_KEY + "&units=metric";

    string weatherData = fetchWeatherData(url);
    if (weatherData.empty()) return;

    Json::Reader reader;
    Json::Value jsonWeather;
    if (!reader.parse(weatherData, jsonWeather)) return;

    time_t timestamp = jsonWeather["dt"].asLargestInt();
    string date = timestampToDate(timestamp);
    string description = jsonWeather["weather"][0]["description"].asString();
    float temp = jsonWeather["main"]["temp"].asFloat();
    int humidity = jsonWeather["main"]["humidity"].asInt();
    float windSpeed = jsonWeather["wind"]["speed"].asFloat();

    cout << "Current Weather:" << endl;
    cout << "Date: " << date << endl;
    cout << "Weather: " << description << endl;
    cout << "Temperature: " << temp << "°C" << endl;
    cout << "Humidity: " << humidity << "%" << endl;
    cout << "Wind Speed: " << windSpeed << " m/s" << endl;
    cout << "--------------------------------------" << endl;
}

// **获取未来 3 天的天气预报**
/*
   选择 Forecast API 的原因：
   1. Forecast API 提供未来 5 天的天气预报，每天 8 个时间点（间隔 3 小时）。
   2. 通过遍历 8 个数据点，可以计算出一天内的 "最高温度" 和 "最低温度"。
   3. 直接使用 Forecast API 提供的 temp_min / temp_max 数据可能不准确，因此需要手动计算。

   计算 min/max 温度的方法：
   1. 遍历 API 返回的 `list`（未来 5 天，每 3 小时的数据）。
   2. 统计每一天的 8 个数据点，计算最小温度和最大温度。
   3. 只显示未来 3 天的数据，不包括今天。

   线程管理：
   - 该函数运行在独立的线程中，每 12 小时自动更新一次天气预报。
*/
void getWeatherForecast() {
    string url = "https://api.openweathermap.org/data/2.5/forecast?q=" + CITY +
                 "&appid=" + API_KEY + "&units=metric";

    string forecastData = fetchWeatherData(url);
    if (forecastData.empty()) return;

    Json::Reader reader;
    Json::Value jsonForecast;
    if (!reader.parse(forecastData, jsonForecast)) return;

    string today = timestampToDate(time(nullptr));
    map<string, pair<float, float>> dailyTemps;
    map<string, string> weatherDescriptions;

    for (const auto& item : jsonForecast["list"]) {
        string date = timestampToDate(item["dt"].asLargestInt());
        if (date <= today) continue;  // 跳过今天的数据
        if (dailyTemps.size() >= 3) break;  // 只保留未来 3 天

        float temp = item["main"]["temp"].asFloat();
        string description = item["weather"][0]["description"].asString();

        // 计算一天内的 min/max 温度
        if (dailyTemps.find(date) == dailyTemps.end()) {
            dailyTemps[date] = {temp, temp};  // 初始化 min/max
            weatherDescriptions[date] = description;
        } else {
            dailyTemps[date].first = min(dailyTemps[date].first, temp);
            dailyTemps[date].second = max(dailyTemps[date].second, temp);
        }
    }

    cout << "3-Day Weather Forecast:" << endl;
    for (const auto& [date, temps] : dailyTemps) {
        cout << "Date: " << date
             << "  Weather: " << weatherDescriptions[date]
             << "  Temperature: " << temps.first << "°C ~ " << temps.second << "°C" << endl;
    }
    cout << "--------------------------------------" << endl;
}

// **线程管理**
/*
   线程设计：
   - `startWeatherUpdates()` 在独立线程中运行，每 30 分钟获取一次当前天气。
   - `startForecastUpdates()` 在独立线程中运行，每 12 小时获取一次天气预报。
   - `main()` 线程不会被阻塞，确保程序持续运行。

 为什么不用计时器？
   - 传统计时器（如 `setTimeout()`）可能阻塞主线程，影响性能。
   - `this_thread::sleep_for()` 让线程自己休眠，不影响主线程运行。
*/

// **定时获取当前天气**
void startWeatherUpdates() {
    while (true) {
        getCurrentWeather();
        this_thread::sleep_for(chrono::minutes(30));
    }
}

// **定时获取天气预报**
void startForecastUpdates() {
    while (true) {
        getWeatherForecast();
        this_thread::sleep_for(chrono::hours(12));
    }
}

// **主函数**
int main() {
    thread weatherThread(startWeatherUpdates);
    thread forecastThread(startForecastUpdates);
    weatherThread.detach();
    forecastThread.detach();

    while (true) {
        this_thread::sleep_for(chrono::hours(1));
    }

    return 0;
}