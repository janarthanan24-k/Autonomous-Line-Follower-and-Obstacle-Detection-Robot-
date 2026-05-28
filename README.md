# Autonomous-Line-Follower-and-Obstacle-Detection-Robot-🤖🏎️

## Overview
This project is an advanced, high-speed line-following robot equipped with an ultrasonic obstacle detection system and a dynamic Human-Machine Interface (HMI). Unlike standard line-followers that rely on basic "bang-bang" control and blocking delays, this rover utilizes a Proportional-Derivative (PD) control loop and asynchronous `millis()` timers to juggle motor control, radar scanning, and live OLED animations without CPU bottlenecking. 

When the path is clear, the robot displays a smiling face and live distance telemetry. If an obstacle is detected within 10cm, the robot executes a hard stop, triggers an active buzzer alarm, and dynamically updates its OLED face to a frown.

## ✨ Key Features
* **High-Speed PD Control:** Calculates track error dynamically for smooth, proportional steering corrections, eliminating speed wobbles.
* **Non-Blocking Radar:** Uses strict `pulseIn()` timeouts (10ms) and asynchronous timers (60ms intervals) to prevent "sonic jamming" and CPU freezing.
* **Interactive HMI (OLED Dashboard):** Acts as a localized display showing live spatial telemetry (Distance in cm), operational state (STRAIGHT, LEFT, HALTED), and a state-based animated face.
* **SRAM Memory Optimization:** Built specifically for the Arduino Nano's 2048-byte memory limit by utilizing lightweight character pointers (`const char*`) instead of `String` objects, preventing memory leaks and crashes during screen rendering.
* **Fail-Safe Interrupts:** Instantly cuts motor power and sounds an active buzzer alarm upon detecting obstacles at the 10cm threshold.

## 🛠️ Hardware Requirements
* **Microcontroller:** Arduino Nano 
* **Motor Driver:** TB6612FNG (or similar dual motor driver)
* **Motors:** 2x DC Gear Motors (BO Motors)
* **Line Sensors:** 3x IR Sensor Array (Left, Center, Right)
* **Radar:** HC-SR04 Ultrasonic Sensor
* **Display:** 0.96" I2C OLED Display
* **Audio:** Active Buzzer
* **Power:** 2x 18650 Li-ion Batteries (or similar high-discharge power source)

## 🔌 Wiring Guide

| Component | Pin / Connection | Arduino Nano Pin |
| :--- | :--- | :--- |
| **IR Sensor Left** | OUT | D2 |
| **IR Sensor Center** | OUT | D3 |
| **IR Sensor Right** | OUT | D4 |
| **Ultrasonic HC-SR04** | TRIG | A0 |
| **Ultrasonic HC-SR04** | ECHO | A1 |
| **OLED Display** | SDA | A4 |
| **OLED Display** | SCL | A5 |
| **Active Buzzer** | Positive (+) | D12 |
| **Motor Driver** | PWMA (Left Speed) | D5 |
| **Motor Driver** | PWMB (Right Speed) | D6 |
| **Motor Driver** | AIN1, AIN2 (Left Dir) | D8, D9 |
| **Motor Driver** | BIN1, BIN2 (Right Dir)| D10, D11 |
| **Motor Driver** | STBY (Standby) | D7 |

## 💻 Software & Libraries
This project requires the Arduino IDE and the following libraries installed via the Library Manager:
* `Wire.h` (Built-in I2C library)
* `Adafruit_GFX.h` (Core graphics library)
* `Adafruit_SSD1306.h` (Hardware-specific library for the OLED)

## ⚙️ How It Works (The Engineering)

### 1. The Control Loop
The robot reads the 3-sensor array to calculate an error value between `-3` and `3`. The PD algorithm applies the formula:
`Correction = (Kp * Error) + (Kd * (Error - LastError))`
This allows the robot to cruise smoothly at a base speed of 140, automatically slowing down the inner wheel on sharp turns to prevent sliding off the track.

### 2. Asynchronous Multitasking
To prevent the slow I2C OLED screen from crashing the motors, the code bypasses standard `delay()` functions. Using `millis()` timers, the robot is separated into strict execution zones:
* **PID Loop:** Runs continuously as fast as the CPU allows.
* **Radar Loop:** Pings the ultrasonic sensor exactly once every 60ms.
* **Animation Engine:** Redraws the OLED face and telemetry HUD exactly once every 150ms.

## 🚀 Installation & Setup
1. Clone this repository to your local machine.
2. Open the `.ino` file in the Arduino IDE.
3. Ensure the Adafruit GFX and SSD1306 libraries are installed.
4. Verify your wiring matches the pinout above. *(Note: The code flips the OLED screen 180 degrees using `display.setRotation(2)` to allow for easier wire routing at the top of the chassis. Remove this line if your screen is mounted normally).*
5. Upload the code to your Arduino Nano.
6. Power on the robot. You will hear a 150ms confirmation beep, followed by a 3-second visual countdown on the built-in LED before the motors engage.

## 📸 Project Gallery

![Robot Front View](LFR/robot-front.jpg)

![Robot in Action](LFR/robot-action.jpg)

## 📈 Future Improvements
* Implementation of a PID Auto-Tuning sequence.
* Adding an avoidance maneuver (Box maneuver) to drive around obstacles and re-acquire the line.
* Upgrading to a 5-sensor IR array for 90-degree intersection handling.

  ## Author
  K. Janarthanan
