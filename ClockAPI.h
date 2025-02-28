#ifndef CLOCK_API_H
#define CLOCK_API_H

#include <string>

class ClockAPI {
public:
    // 获取系统当前时间
    static std::string getSystemTime();

    // 本地时钟上报（返回当前时间字符串）
    static std::string reportLocalTime();

    // 触发联网时间同步
    static bool syncTimeWithNTP();
};

#endif // CLOCK_API_H


