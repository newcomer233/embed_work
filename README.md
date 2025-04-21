

# PiGrid: Voice & Gesture Controlled Real-Time Smart Display

## 1. Introduction

PiGrid is an interactive, pixel-style smart display built on Raspberry Pi 5 and driven by MAX7219 LED matrices. As a project for the Real Time Embedded Programming course (ENG5220), this device features time/weather display, timer, and snake game, with control based on voice input via bluetooth microphone and gesture recognition using PAJ7620u2 and MPU6050. With its combination of display, sensors, and processing capabilities, PiGrid could be extended into various smart home roles, such as a personalized status display, notification center, or even a simple controller.

## 2. Demo & Usage

**Demo Video:** [https://www.youtube.com/watch?v=_KGyjrOgzdA](https://www.youtube.com/watch?v=_KGyjrOgzdA)

PiGrid boots up automatically when powered on, starting with the Time display. Here's how to interact with its different modes:

### Mode Switching 

**Action:** Tap the **MPU6050** sensor module.  
**Result:** Cycles through the available modes sequentially (e.g., Time -> Game -> Counter -> Weather -> Time...).  

### Modes & Voice Commands

Activate modes and features using the following voice commands:  

*   **Command:** "time"  
**Action:** Switches to the **Time Display** mode, showing the current time.   

*   **Command:** "counter"   
**Action:** Switches to **Countdown Timer** mode (display likely prompts for input).  
**Controls:** Say four digits (e.g., "one two three four") to set the duration (interpreted as 12 minutes and 34 seconds) and start the countdown automatically.  

*   **Command:** "weather"  
**Action:** Switches to **Weather Display** mode.    
**Details:** Shows the current weather conditions (icon) and temperature (°C) based on data fetched from **OpenWeather**.  

*   **Command:** "game"  
**Action:** Launches the **Snake Game**.    
**Controls:** Use voice commands ("up", "down", "left", "right") **or** swipe your hand (up, down, left, right) across the **PAJ7620U2** sensor to control the snake's direction.  

## 3. Hardware Requirements

*   **Raspberry Pi 5** (4GB/8GB RAM) x 1
*   **MAX7219** 8x8 LED Dot Matrix Module x 2 
*   **PAJ7620U2** Gesture Recognition Sensor Module x 1
*   **MPU6050** Gyroscope/Accelerometer Module x 1
*   **Bluetooth Earphone/Headset with Microphone** x 1 

###  Wiring Overview

| Device        | Interface | RPi Bus/Pin                | Note                                  |
|---------------|-----------|-----------------------------|---------------------------------------|
| **MPU6050**   | I2C       | `I2C-1` (`/dev/i2c-1`)      | Address `0x68`, INT → GPIO 13         |
| **PAJ7620U2** | I2C       | `I2C-3` (`/dev/i2c-3`)      | Address `0x73`, INT → GPIO 12         |
| **MAX7219**   | SPI       | `SPI0.0` (`/dev/spidev0.0`) | MOSI: GPIO 10, CLK: GPIO 11, CS: GPIO 8 |

---

###  Wiring Details

####  MPU6050 (I2C-1)
- `VCC` → 3.3V / 5V  
- `GND` → GND  
- `SCL` → GPIO 3 (I2C-1 SCL)  
- `SDA` → GPIO 2 (I2C-1 SDA)  
- `INT` → GPIO 13  

####  PAJ7620U2 (I2C-3)
- `VCC` → 3.3V / 5V  
- `GND` → GND  
- `SCL` → I2C-3 SCL 
- `SDA` → I2C-3 SDA  
- `INT` → GPIO 12  

> I2C-3 is not enabled by default on Raspberry Pi. You may need to configure it using a device tree overlay or software I2C.

####  MAX7219 (SPI0.0)
- `VCC` → 5V  
- `GND` → GND  
- `DIN` → GPIO 10 (SPI MOSI)  
- `CLK` → GPIO 11 (SPI SCLK)  
- `CS`  → GPIO 8 (SPI CE0)

## 4. Software Requirements

The following tools, libraries, and components are required to build and run PiGrid.  
All dependencies are installed automatically via the provided Dockerfile.  
Alternatively, they can be installed manually as listed below:

###  Build Tools

- `g++` – GNU C++ compiler  
- `cmake` – Cross-platform build system generator

###  System Libraries (via APT)

- `libgpiod-dev` – For GPIO event handling (used by PAJ7620U2 and MPU6050)
- `libcurl4-openssl-dev` – Used for fetching live weather data via OpenWeather API
- `libjsoncpp-dev` – Used for parsing OpenWeather API JSON responses
- `espeak-ng` – Lightweight speech synthesis backend (for voice response)

Install them manually (if needed):

```bash
sudo apt update
sudo apt install g++ cmake libgpiod-dev libcurl4-openssl-dev libjsoncpp-dev espeak-ng
```
Following open-source projects were used in our system, allowing performimg real-time local speech recognition and synthesis without relying on cloud services:

VOSK - A lightweight and accurate offline speech recognition toolkit.
  https://github.com/alphacep/vosk-api

Piper - A fast and high-quality neural text-to-speech (TTS) system optimized for local execution, especially on embedded devices like the Raspberry Pi.
  https://github.com/rhasspy/piper

Sometimes, directly pulling from this project may cause issues with onnxruntime or piper-phonemize_linux_aarch64, potentially leaving them broken. A .zip file containing these libraries is provided in the lib/ directory.   
Also, you can download them from the official ONNX Runtime website or the Piper project.

## 5. Division of Labour

**Zehua Xu:** Project Integration, Sensor Interfacing & Debugging  
**Huinan Guo:** Weather API Integration & Data Processing  
**Keji Lu:** Voice Recognition Module Implementation  
**Linfeng Zhang:** Visual Design, Display Implementation, PR  
**Mengxiang Duan:** System Interaction Logic Design  
**Jinhao Xu:** Hardware Modification, Assembly, and Exterior Design


