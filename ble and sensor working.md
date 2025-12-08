#include <Servo.h>

#include "Arduino_LED_Matrix.h"

#include <ArduinoBLE.h>

  

// Initialize Objects

Servo fanESC;

ArduinoLEDMatrix matrix;

  

// --- Pin Definitions ---

const int ESC_PIN = 9;

const int TRIG_PIN = 5;

const int ECHO_PIN = 6;

  

// --- Sensor & Control Constants ---

const float SPEED_OF_SOUND = 0.0343;

const int LAUNCH_THRESHOLD_CM = 3.1; // Fan ON if distance > 10 cm or Out of Range

const int SHUTDOWN_THRESHOLD_CM = 3; // Fan OFF if distance <= 9 cm

  

// --- CRITICAL THROTTLE CONSTANTS ---

const int THROTTLE_MIN = 1000; // 0% throttle (Arming and Stopping)

const int THROTTLE_MAX = 2000; // 100% throttle

  

// --- State Variables (Defined globally) ---

bool isArmed = false;

bool sensor_enabled = false;

bool isFanRunning = false;

int blink_state = 0;

  

// Track the current mode for change detection

enum Mode { MODE_DISARMED, MODE_MANUAL_READY, MODE_FAN_ON, MODE_SENSOR_READY };

Mode current_mode = MODE_DISARMED;

  

// --- BLE Definitions (Defined globally) ---

BLEService carService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLEIntCharacteristic startCommandCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

BLEIntCharacteristic modeCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

  

// --- LED Matrix Patterns (Defined globally) ---

uint8_t BLE_CONNECTED_PATTERN[8][12] = {

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },

{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },

{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },

{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }

};

uint8_t FAN_ON_PATTERN[8][12] = {

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }

};

uint8_t SWITCH_PATTERN[8][12] = {

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

};

uint8_t DISARMED_DOT[8][12] = {

{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },

{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

};

  
  

// ====================================

// CORE FUNCTIONS

// ====================================

  

float getDistance_cm() {

digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);

digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);

digitalWrite(TRIG_PIN, LOW);

long duration = pulseIn(ECHO_PIN, HIGH, 30000);

if (duration == 0) return 400.0;

return (duration * SPEED_OF_SOUND) / 2.0;

}

  

void setFanPower(bool state) {

if (state == true) {

if (!isFanRunning) {

fanESC.writeMicroseconds(THROTTLE_MAX);

isFanRunning = true;

Serial.println("COMMAND: FAN ACTIVATED (2000us)");

}

} else {

if (isFanRunning) {

fanESC.writeMicroseconds(THROTTLE_MIN);

isFanRunning = false;

Serial.println("COMMAND: FAN SHUTDOWN (1000us)");

}

}

}

  

// ====================================

// DEBUG & DISPLAY FUNCTION

// ====================================

  

void displayStatus(float dist_cm, bool ble_connected) {

// 1. DETERMINE NEW MODE

Mode new_mode;

if (!isArmed) { new_mode = MODE_DISARMED; }

else if (isFanRunning) { new_mode = MODE_FAN_ON; }

else if (sensor_enabled) { new_mode = MODE_SENSOR_READY; }

else { new_mode = MODE_MANUAL_READY; }

  

// 2. DETECT SWITCH AND NOTIFY USER

if (new_mode != current_mode) {

current_mode = new_mode;

Serial.print("--------------------------------");

Serial.print("\nMODE SWITCHED TO: ");

if (new_mode == MODE_DISARMED) Serial.println("DISARMED");

else if (new_mode == MODE_FAN_ON) Serial.println("FAN IS RUNNING");

else if (new_mode == MODE_SENSOR_READY) Serial.println("SENSOR ACTIVE / FAN OFF");

else if (new_mode == MODE_MANUAL_READY) Serial.println("MANUAL READY / FAN OFF");

Serial.println("--------------------------------");

matrix.renderBitmap(SWITCH_PATTERN, 8, 12);

delay(50);

}

  

// 3. CONTINUOUS STATUS DISPLAY

blink_state = (blink_state + 1) % 10;

  

Serial.print("Status: ");

Serial.print(current_mode == MODE_DISARMED ? "DISARMED" : (isFanRunning ? "FAN ON" : "ARMED, FAN OFF"));

if (sensor_enabled) {

Serial.print(" | Dist: ");

if (dist_cm >= 400.0) { Serial.println("Out of Range"); }

else { Serial.print(dist_cm); Serial.println(" cm"); }

} else {

Serial.println(" | Sensor: Disabled");

}

  

// LED Matrix: Continuous Mode Indicator

if (current_mode == MODE_FAN_ON) { matrix.renderBitmap(FAN_ON_PATTERN, 8, 12); }

else if (current_mode == MODE_MANUAL_READY || current_mode == MODE_SENSOR_READY) {

matrix.renderBitmap(BLE_CONNECTED_PATTERN, 8, 12);

} else if (current_mode == MODE_DISARMED) {

if (blink_state > 5) { matrix.renderBitmap(DISARMED_DOT, 8, 12); }

else { matrix.clear(); }

}

}

  
  

// ====================================

// SETUP FUNCTION

// ====================================

void setup() {

Serial.begin(9600);

matrix.begin();

matrix.clear();

pinMode(TRIG_PIN, OUTPUT);

pinMode(ECHO_PIN, INPUT);

  

// --- BLE Setup (CRITICAL FIX: Initialize BLE first) ---

if (!BLE.begin()) {

Serial.println("!!! FATAL ERROR: BLE initialization failed! Program Halted. !!!");

while (1);

}

// --- ESC Attach (Attach after BLE succeeds) ---

fanESC.attach(ESC_PIN);

// Attach BLE characteristics to service

BLE.setAdvertisedService(carService);

carService.addCharacteristic(startCommandCharacteristic);

BLE.addService(carService);

carService.addCharacteristic(modeCharacteristic);

BLE.addService(carService);

BLE.setLocalName("DerbyCarV5");

Serial.println("System Initialized. Awaiting BLE Connection to Arm ESC.");

current_mode = MODE_DISARMED;

BLE.advertise();

}

  

// ====================================

// LOOP FUNCTION (DIRECT SENSOR CONTROL)

// ====================================

void loop() {

BLEDevice central = BLE.central();

bool ble_connected = central;

float distance = 0.0;

  

// --- A. BLE-REQUIRED ARMING/DISARMING ---

if (ble_connected && !isArmed) {

Serial.println("\n--- BLE CONNECTED: ARMING ESC NOW (ESC Must Beep) ---");

fanESC.writeMicroseconds(THROTTLE_MIN);

isArmed = true;

delay(100);

}

if (!ble_connected && isArmed) {

Serial.println("\n--- BLE DISCONNECTED: DISARMING CAR ---");

setFanPower(false); // Shut down fan

isArmed = false;

current_mode = MODE_DISARMED;

}

// --- B. BLE COMMAND HANDLING ---

if (isArmed) {

// 1. CHECK FOR MODE CHANGE COMMAND (Fixes Mode Switch Issue)

if (modeCharacteristic.written()) {

int mode_cmd = modeCharacteristic.value();

if (mode_cmd == 2) {

sensor_enabled = true;

Serial.println("Mode set to SENSOR AUTO-TRIGGER.");

} else {

sensor_enabled = false;

Serial.println("Mode set to MANUAL TRIGGER.");

}

modeCharacteristic.writeValue(mode_cmd); // Acknowledge write

}

// 2. CHECK FOR FAN ON/OFF COMMAND (Manual override for testing)

if (startCommandCharacteristic.written()) {

int command = startCommandCharacteristic.value();

if (command == 1) { setFanPower(true); } else if (command == 0) { setFanPower(false); }

startCommandCharacteristic.writeValue(command);

}

}

  

// --- C. DIRECT SENSOR CONTROL LOGIC (MODIFIED FOR OUT OF RANGE) ---

if (isArmed && sensor_enabled) {

distance = getDistance_cm();

// 1. SAFETY SHUTDOWN CHECK: Turn fan OFF if distance is <= 9 cm

if (distance <= SHUTDOWN_THRESHOLD_CM) {

if (isFanRunning) {

Serial.println("SENSOR SHUTDOWN: Obstacle <= 9cm. FAN OFF.");

}

setFanPower(false);

}

// 2. LAUNCH/KEEP RUNNING CHECK: Turn fan ON if distance > 10 cm OR out of range (400cm)

else if (distance > LAUNCH_THRESHOLD_CM || distance == 400.0) {

if (!isFanRunning) {

Serial.println("SENSOR TRIGGER/RUNNING: Distance > 10cm or Out of Range. FAN ON.");

}

setFanPower(true);

}

// 3. IDLE ZONE (Between 9.01 cm and 10 cm): Fan remains OFF by default.

}

// D. Display current status

displayStatus(distance, ble_connected);

delay(50);

}