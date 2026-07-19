# esp32-ultrasonic-oled-radar
# ESP32 Multi-Zone Ultrasonic Radar System 🚀

An interactive multi-zone proximity monitoring system simulated in Wokwi using an ESP32. The system tracks distances across 6 individual zones, alerts the user visually and audibly to obstacles, and monitors device orientation via an IMU sensor.

---

## ⚡ Live Interactive Simulation
You can launch, inspect, and run this entire circuit directly in your browser without installing any software:

👉 **[Click Here to Run the Simulation on Wokwi](https://wokwi.com/projects/from/github/Maithra08/esp32-ultrasonic-oled-radar)**

---

## 🛠️ Components List
* **Microcontroller:** ESP32 DevKit C V4
* **Proximity Sensors:** 6x HC-SR04 Ultrasonic Distance Sensors (Front, Front Left, Front Right, Rear, Rear Left, Rear Right)
* **Visual Display:** SSD1306 128x64 I2C OLED Screen
* **Telemetry Sensor:** MPU6050 6-Axis Motion Tracking IMU (Accelerometer & Gyro)
* **Indicators:** 6x Common-Cathode RGB LEDs driven by a 2x Cascaded 74HC595 Shift Register array
* **Audio Alert:** Active Piezo Buzzer

---

## 🧠 How It Works
1. **Scanning Phase:** The ESP32 continuously loops through and fires all 6 ultrasonic sensors to monitor the surrounding space.
2. **Safe State:** When all zones are clear (distance greater than 30 cm), the OLED displays live tilt telemetry (X and Y acceleration) from the MPU6050, and the RGB indicators stay solid **Green**.
3. **Danger Alarm Phase:** If any sensor detects an obstacle closer than 30 cm:
   * The buzzer sounds a continuous alert tone.
   * The OLED instantly clears out and draws a geometric **Warning Triangle Symbol** along with text identifying the specific hazard zone (e.g., `ALARM: FL SENSOR`) and the exact distance.
   * The respective RGB indicator lights flip immediately to bright **Red**.
