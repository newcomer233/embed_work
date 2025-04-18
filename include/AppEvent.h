// AppEvent.h
#pragma once

enum class AppEvent {
    SwitchMode,      // switch mode like loop
    EnterTime,       // time mode
    EnterGame,       // game mode
    EnterCounter,     // counter mode
    EnterWeather,     // wheather mode
    VoiceDigitsReady  // identify 4digital bits
};