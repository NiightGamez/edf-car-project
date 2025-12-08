# ⚡ Wiring and Power Flow Diagram

**⚠️ WARNING:** The LiPo battery supplies 11.1V. Use caution. Ensure the **Fan Motor is disconnected** from the ESC during initial setup and testing. The ESC's BEC provides safe 5V power to the Arduino.

## 🔌 Signal and Control Connections

| Component | Pin (Wire Color) | Arduino Uno R4 Pin | Notes |
| :--- | :--- | :--- | :--- |
| **ESC Signal** | Signal (White/Yellow) | **D9** | PWM output for throttle control. |
| **Ultrasonic Trigger** | Trig | **D5** | Sends the sonic pulse. |
| **Ultrasonic Echo** | Echo | **D6** | Receives the reflected pulse. |

## 🔋 Power Connections

| Component | Pin (Wire Color) | Arduino Uno R4 Pin | Voltage |
| :--- | :--- | :--- | :--- |
| **LiPo Battery** (11.1V) | Large Red/Black | Connects ONLY to **30A ESC Power Input** | High Power |
| **ESC BEC Output** | Red | **5V Pin** | Powers the Arduino. |
| **ESC BEC Output** | Black/Brown | **GND Pin** | Common Ground. |



## Power Flow Summary

1.  **LiPo** $\rightarrow$ **ESC Power Input** (11.1V)
2.  **ESC** $\rightarrow$ **Ducted Fan Motor** (Controls motor speed)
3.  **ESC BEC** $\rightarrow$ **Arduino 5V/GND** (5V regulated power)
4.  **Arduino** $\rightarrow$ **HC-SR04** (Powered by 5V rail)