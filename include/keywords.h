// keywords.h
#pragma once
#include <unordered_map>
#include <string>

inline const std::unordered_map<std::string, std::string> keywordMap = {
    {"up", "UP"},
    {"down", "DOWN"},
    {"left", "LEFT"},
    {"right", "RIGHT"},
    {"mode", "MODE"},
    // 你可以在这里扩展更多别名：
    {"上", "UP"},
    {"下", "DOWN"},
    {"左", "LEFT"},
    {"右", "RIGHT"},
    {"模式", "MODE"},
    // some word we test to matching the test
    {"oh","DOWN"},
    {"tom", "DOWN"},
    {"ha", "DOWN"}
};
