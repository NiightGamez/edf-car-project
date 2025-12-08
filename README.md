# 🚀 Pinewood Derby Ducted Fan Car Control System

This system uses an **Arduino Uno R4 WiFi** board as the master controller for a ducted fan Pinewood Derby car. It features BLE-required arming for safety, dual trigger modes (Ultrasonic Auto-Start or BLE Manual Trigger), and real-time status feedback via the onboard LED Matrix and Serial Monitor.

## 📋 Parts List

| Component | Function | Status |
| :--- | :--- | :--- |
| **Microcontroller** | Arduino Uno R4 WiFi (or Freenove V5) | **Have** |
| **Trigger Sensor** | HC-SR04 Ultrasonic Sensor | **Have** |
| **Motor Controller** | 30A Brushless Electronic Speed Controller (ESC) | **Pending** |
| **Propulsion** | 30mm Ducted Fan Unit (Brushless Motor) | **Pending** |
| **Power** | 3S 11.1V 1500mAh LiPo Battery | **Pending** |
## ✅Things to do
- Add more functionality while using BLE
- External Monitoring (Be able to view serial monitor and output wirelessly)
- Get components wired up ++ test prior to full build
- Customize LED Matrix (If time permits)
- More controlled fan curve for different modes (control RPMs better)
- Change back to quick sensor data rate (less delay currently at 3 secs)
## 💡 System Logic

1.  **Safety Master Key:** The system is completely inert (**DISARMED**) until a phone connects via BLE.
2.  **Arming:** Connecting via BLE sends the `THROTTLE_MIN` pulse to the ESC, arming the motor controller and enabling the trigger system.
3.  **Mode Select:** The BLE connection allows the user to switch between **MANUAL TRIGGER** and **SENSOR AUTO-TRIGGER** mode.
4.  **Race:** When triggered, the system simulates a full throttle run for 3 seconds.

## 🛠️ Initial Setup Instructions

1.  **Install Libraries:** In the Arduino IDE, install the following libraries:
    * `Servo` (Standard library)
    * `Arduino_LED_Matrix` (For the onboard matrix)
    * `ArduinoBLE` (For Bluetooth Low Energy control)

2.  **Upload Code:** Copy the contents of edf_car.ino` and upload it to your Arduino board.

3.  **BLE Connection (Master Key):**
    * Download **nRF Connect** on your Android phone.
    * Connect the battery to the ESC.
    * Scan for the device **`DerbyCarV5`** and connect. The system will switch from **DISARMED** (Matrix clear) to **MANUAL READY** (Matrix full border).

4.  **Mode Switching via BLE:**
    * Find the **Mode Characteristic** (UUID ending in `...0002`).
    * **Manual Trigger Mode:** Write Hex value **`01`**. (Default mode).
    * **Sensor Auto-Trigger Mode:** Write Hex value **`02`**.