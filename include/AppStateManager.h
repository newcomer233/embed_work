#pragma once
#include <functional>
#include <map>
#include <vector>
#include <iostream>
#include <mutex>

enum class AppState {
    TIME,
    GAME,
    COUNTER,
    WEATHER
};

class AppStateManager {
public:
    AppStateManager();

    AppState getState() const;
    void setState(AppState newState);

    void onEnter(AppState state, std::function<void()> callback);
    void onExit(AppState state, std::function<void()> callback);

private:
    mutable std::mutex stateMutex;
    AppState currentState;
    std::map<AppState, std::vector<std::function<void()>>> enterCallbacks;
    std::map<AppState, std::vector<std::function<void()>>> exitCallbacks;
};