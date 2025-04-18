#include "AppStateManager.h"
#include <mutex>

AppStateManager::AppStateManager() : currentState(AppState::TIME) {}

AppState AppStateManager::getState() const {
    std::lock_guard<std::mutex> lock(stateMutex);
    return currentState;
}

void AppStateManager::setState(AppState newState) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (newState == currentState) return;

    std::cout << "[AppStateManager] State Switch "
              << static_cast<int>(currentState)
              << " -> "
              << static_cast<int>(newState)
              << std::endl;

    for (const auto& cb : exitCallbacks[currentState]) {
        cb();
    }

    currentState = newState;

    for (const auto& cb : enterCallbacks[currentState]) {
        cb();
    }
}

void AppStateManager::onEnter(AppState state, std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(stateMutex);
    enterCallbacks[state].push_back(callback);
}

void AppStateManager::onExit(AppState state, std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(stateMutex);
    exitCallbacks[state].push_back(callback);
}