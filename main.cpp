#include <iostream>
#include "max7219.h"
#include "app_controller.h"

int main() {
    MAX7219 max7219;
    AppController app(max7219);

    app.setTimerFinishedCallback([]() {
        std::cout << "[Main] Timer finished" << std::endl;
    });

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "quit") break;
        app.handleCommand(input);
    }

    app.shutdown();
    max7219.clearDisplayAll();
    return 0;
}


/*
Command Summary:

"time"   : Show system time with colon blinking
"temp"   : Weather mode, input weather code (e.g. s12 / r-5)
"timer"  : Timer mode, input 4-digit countdown time (MMSS)
"p"      : Pause/resume timer
"quit"   : Exit program (valid in any mode)

Weather codes:
s/r/h/l/c/n : Sun / Rain / Heavy Rain / Lightning / Cloud / Snow
Number of temperature, range -9 to 99

Timer mode:
Enter 4-digit countdown time (MMSS, e.g. 0230 = 2 min 30 sec)  
Updates every second, "p" to pause/resume  
At 0000, blinks and returns to input state  
"temp"/"time" can interrupt and switch mode  
*/
