#include "ClockAPI.h"
#include <iostream>

using namespace std;

int main() {
    cout << "Getting system time: " << ClockAPI::getSystemTime() << endl;

    cout << "Reporting local time: " << ClockAPI::reportLocalTime() << endl;

    cout << "Synchronizing time with NTP..." << endl;
    if (ClockAPI::syncTimeWithNTP()) {
        cout << "Time sync successful!" << endl;
    } else {
        cout << "Time sync failed!" << endl;
    }

    return 0;
}
