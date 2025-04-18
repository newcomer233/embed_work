#pragma once
#include "SensorCtrl.h"
#include "snake_game.h"
#include "AppStateManager.h"
#include <iostream>

class GestureHandler_Event : public GestureHandler {
public:
    GestureHandler_Event(SnakeGame& game, AppStateManager& stateMgr)
        : snake(game), stateManager(stateMgr) {}

    void onUp() override {
        if (stateManager.getState() == AppState::GAME)
            snake.setDirection(Direction::LEFT);
    }
    void onDown() override {
        if (stateManager.getState() == AppState::GAME)
            snake.setDirection(Direction::RIGHT);
    }
    void onLeft() override {
        if (stateManager.getState() == AppState::GAME)
            snake.setDirection(Direction::UP);
    }
    void onRight() override {
        if (stateManager.getState() == AppState::GAME)
            snake.setDirection(Direction::DOWN);
    }

private:
    SnakeGame& snake;
    AppStateManager& stateManager;
};