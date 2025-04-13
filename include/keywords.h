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
    // some other name we can add in here to easy manager
    {"上", "UP"},
    {"下", "DOWN"},
    {"左", "LEFT"},
    {"右", "RIGHT"},
    {"模式", "MODE"}
};
