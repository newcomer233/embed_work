#include "AppStateMachine.h"
#include <mutex>

AppStateMachine::AppStateMachine(AppStateManager& mgr) : stateManager(mgr) {}

void AppStateMachine::handleEvent(AppEvent event) {
    std::lock_guard<std::mutex> lock(machineMutex);
    std::cout << "[AppStateMachine] Event: " << static_cast<int>(event) << std::endl;

    switch (event) {
        case AppEvent::SwitchMode: {
            AppState current = stateManager.getState();
            AppState next;
            switch (current) {
                case AppState::TIME:     next = AppState::GAME; break;
                case AppState::GAME:     next = AppState::COUNTER; break;
                case AppState::COUNTER:  next = AppState::WEATHER; break;
                case AppState::WEATHER:  next = AppState::TIME; break;

                default:                 next = AppState::TIME; break;
            }

            stateManager.setState(next);
            break;
        }
        case AppEvent::EnterTime:
            stateManager.setState(AppState::TIME);
            break;
            
        case AppEvent::EnterGame:
            stateManager.setState(AppState::GAME);
            break;

        case AppEvent::EnterCounter:
            stateManager.setState(AppState::COUNTER);
            break;

        case AppEvent::EnterWeather:
            stateManager.setState(AppState::WEATHER);
            break;

        case AppEvent::VoiceDigitsReady:
            // Remain for else
            break;

        default:
            std::cerr << "[AppStateMachine] Unknown Event" << std::endl;
    }
}