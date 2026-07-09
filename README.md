# Smart Security IoT System

Smart Security IoT System is an Arduino and ESP8266 based security project.  
The project simulates a smart home alarm system with keypad control, distance-based movement detection, LCD display feedback, LED indicators, buzzer alerts, servo door lock simulation, and a web dashboard.

The system is split into two main parts:

1. **Arduino Alarm System** – controls the alarm logic, keypad, LCD screen, LEDs, buzzer, ultrasonic sensor, and servo lock.
2. **ESP8266 Web Dashboard** – provides a web interface for monitoring alarm status, movement detection, temperature, humidity, movement history, and chart data.

This project was created as an Embedded Systems and IoT portfolio project to show practical skills in Arduino programming, sensor integration, web-based IoT monitoring, and communication between microcontrollers.
<img width="369" height="622" alt="image" src="https://github.com/user-attachments/assets/64e1d503-00f8-44db-bd73-b43990a1a297" />
Final physical prototype of my Smart Security and Environmental Monitoring System.

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
<img width="1005" height="521" alt="image" src="https://github.com/user-attachments/assets/30766930-c473-49e1-b44c-099f06844d1b" />
<img width="940" height="497" alt="image" src="https://github.com/user-attachments/assets/7172c57e-d3c9-465d-98ae-06b99a05faeb" />
ESP8266 web dashboard showing live system data and movement history.

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
<img width="1000" height="527" alt="image" src="https://github.com/user-attachments/assets/c734a6ad-dd96-4ba4-a5b0-a249e2193c30" />
Wokwi simulation 1 showing ESP with DHT22 and OLED display.
<img width="961" height="505" alt="image" src="https://github.com/user-attachments/assets/5777b4bb-9b86-4d5c-b1f4-5526c44c657c" />
Wokwi simulation 2 showing the Arduino alarm subsystem.
<img width="985" height="519" alt="image" src="https://github.com/user-attachments/assets/8e9f11b9-d866-4992-8a3f-a75386c3cff4" />
Wokwi simulation 3 showing the complete integrated system.
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
<img width="367" height="793" alt="image" src="https://github.com/user-attachments/assets/e1ff8c19-dae2-41da-9181-824f7ca44f30" />
<img width="373" height="807" alt="image" src="https://github.com/user-attachments/assets/4f152b53-b859-41d1-8764-64e9eca7f765" />

Arduino UNO alarm subsystem with LCD, keypad, HC-SR04 sensor, servo motor, LEDs and buzzer. And an ESP8266 IoT module with an OLED display and a DHT sensor.
---

## Project Structure

Smart-Security-IoT/
│
├── Arduino_Alarm_System.ino
├── ESP8266_Web_Dashboard.ino
└── README.md
