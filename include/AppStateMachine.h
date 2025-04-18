#pragma once

#include "AppStateManager.h"
#include "AppEvent.h"
#include <iostream>
#include <mutex>  

class AppStateMachine {
public:
    
    AppStateMachine(AppStateManager& mgr);

    
    void handleEvent(AppEvent event);

private:
    AppStateManager& stateManager;
    std::mutex machineMutex;  
};