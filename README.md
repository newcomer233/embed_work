# embed_work
for UoG Team 27 embed work
#
This project and its source code are part of the coursework for ENG5220: Real-Time Embedded Programming (2024–25) at the University of Glasgow.
Until the project is officially completed, in principle, the code is not available for use outside UoG Team 27.

PS: Honestly, would anyone even want code from students like us at the bottom of the food chain?
Even feeding it to an AI might get a "meh, too noob" response (tears).

PPS: On second thought... maybe we should feed it to an AI—to help spot AI's bugs! Haha.

2025_04_14_01:02

I finally know why told us don't use GPT4..when translate PPS sentence to English "maybe we should feed it to an AI—to help spot bugs!"  emmmm, AI still know nothing about humans thinking.



We utilize the following open-source projects in our system:

VOSK - A lightweight and accurate offline speech recognition toolkit.
  https://github.com/alphacep/vosk-api

Piper - A fast and high-quality neural text-to-speech (TTS) system optimized for local execution, especially on embedded devices like the Raspberry Pi.
  https://github.com/rhasspy/piper

These components enable our system to perform real-time, local speech recognition and synthesis without relying on cloud services.

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
*   **Jumper Wires** x  

*(wiring diagram)*

## 4. Division of Labour

**Zehua Xu:** Project Integration, Sensor Interfacing & Debugging  
**Huinan Guo:** Weather API Integration & Data Processing  
**Keji Lu:** Voice Recognition Module Implementation  
**Linfeng Zhang:** Visual Design, Display Implementation, PR  
**Mengxiang Duan:** System Interaction Logic Design  
**Jinhao Xu:** Hardware Modification, Assembly, and Exterior Design


