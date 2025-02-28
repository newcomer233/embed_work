#include "ClockAPI.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <iomanip>

using namespace std;

// 获取当前系统时间
string ClockAPI::getSystemTime() {
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    struct tm *timeinfo = localtime(&now_c);
    stringstream ss;
    ss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 返回当前时间，模拟本地时钟上报
string ClockAPI::reportLocalTime() {
    string currentTime = getSystemTime();
    cout << "[ClockAPI] Local Time: " << currentTime << endl;
    return currentTime;
}

// 触发联网时间同步
bool ClockAPI::syncTimeWithNTP() {
    cout << "[ClockAPI] Synchronizing time with NTP server..." << endl;
    int ret = system("sudo sntp -sS time.google.com"); // 使用 Google NTP 服务器
    if (ret == 0) {
        cout << "[ClockAPI] Time successfully synchronized!" << endl;
        return true;
    } else {
        cerr << "[ClockAPI] Failed to synchronize time." << endl;
        return false;
    }
}
