# Smart Security IoT System

Smart Security IoT System is an Arduino and ESP8266 based security project.  
The project simulates a smart home alarm system with keypad control, distance-based movement detection, LCD display feedback, LED indicators, buzzer alerts, servo door lock simulation, and a web dashboard.

The system is split into two main parts:

1. **Arduino Alarm System** – controls the alarm logic, keypad, LCD screen, LEDs, buzzer, ultrasonic sensor, and servo lock.
2. **ESP8266 Web Dashboard** – provides a web interface for monitoring alarm status, movement detection, temperature, humidity, movement history, and chart data.

This project was created as an Embedded Systems and IoT portfolio project to show practical skills in Arduino programming, sensor integration, web-based IoT monitoring, and communication between microcontrollers.

---

## Project Overview

The main goal of this project was to build a working smart security prototype.  
The Arduino handles the physical alarm system, while the ESP8266 creates a web dashboard that can be opened in a browser.

The system can:

- Arm the alarm system.
- Disarm the alarm using a PIN code.
- Detect movement using an ultrasonic sensor.
- Trigger a buzzer and red LED when movement is detected.
- Use a servo motor to simulate a door lock.
- Display alarm status on an LCD screen.
- Send alarm and movement signals to the ESP8266.
- Show live system data on a web dashboard.
- Display temperature and humidity from a DHT11 sensor.
- Save movement history.
- Show simple temperature and humidity charts.
- Allow the user to change the movement logging interval from the web dashboard.

---

## Features

### Arduino Alarm System

- Keypad PIN control
- Alarm armed and disarmed modes
- Ultrasonic distance detection
- LCD 16x2 I2C display
- Green LED for safe/disarmed status
- Red LED for alarm/armed warning status
- Buzzer alarm sound
- Servo motor door lock simulation
- Movement detection cooldown system
- Communication signals sent to ESP8266
- Movement interval command received from ESP8266

### ESP8266 Web Dashboard

- Web dashboard hosted by ESP8266
- Live alarm status
- Live movement status
- Temperature and humidity readings using DHT11
- OLED display support
- Movement history log
- Clear movement history button
- Dark and light mode toggle
- Temperature and humidity chart
- Movement history interval selection
- Wi-Fi station mode
- Backup access point mode if Wi-Fi connection fails

---

## Hardware Used

The project uses the following components:

- Arduino UNO
- ESP8266
- HC-SR04 ultrasonic sensor
- 4x3 keypad
- LCD 16x2 I2C display
- OLED I2C display
- DHT11 temperature and humidity sensor
- Servo motor
- Green LED
- Red LED
- Buzzer
- Breadboard
- Jumper wires
- Resistors

---

## Technologies Used

- Arduino IDE
- Arduino C/C++
- ESP8266 Wi-Fi programming
- HTML
- CSS
- JavaScript
- Embedded systems programming
- Web server on ESP8266
- I2C communication
- Digital input and output
- Sensor integration

---

## Project Structure

Smart-Security-IoT/
│
├── Arduino_Alarm_System.ino
├── ESP8266_Web_Dashboard.ino
└── README.md
