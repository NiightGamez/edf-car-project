#include "Arduino_LED_Matrix.h"
#include <ArduinoBLE.h>

// Initialize Objects
ArduinoLEDMatrix matrix;

// --- Pin Definitions ---
const int TRIG_PIN = 5; 
const int ECHO_PIN = 6; 

// --- Sensor & Test Constants ---
const float SPEED_OF_SOUND = 0.0343;
const int STAGING_THRESHOLD_CM = 10; 
const int BLINK_DELAY_MS = 50; // Main loop delay

// --- State Variables ---
bool isArmed = false; 
bool sensor_enabled = false; // False by default (Manual Mode)
int blink_state = 0; 

// Track the current mode for change detection
enum Mode { MODE_DISARMED, MODE_MANUAL_READY, MODE_SENSOR_IDLE, MODE_SENSOR_STAGED };
Mode current_mode = MODE_DISARMED; 

// --- BLE Definitions ---
// Uses the same UUIDs as the final code
BLEService carService("19B10000-E8F2-537E-4F6C-D104768A1214"); 
BLEIntCharacteristic startCommandCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); 
BLEIntCharacteristic modeCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite); 


// --- LED Matrix Patterns (UNCHANGED) ---
// STAGING Pattern (Inner box)
uint8_t STAGED_PATTERN[8][12] = { /* ... */ };
// BLE CONNECTED Pattern (Full border)
uint8_t BLE_CONNECTED_PATTERN[8][12] = { /* ... */ };
// SWITCH INDICATOR Pattern (Horizontal lines)
uint8_t SWITCH_PATTERN[8][12] = { /* ... */ };


// ====================================
// CORE FUNCTIONS 
// ====================================

float getDistance_cm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  if (duration == 0) return 400.0;
  return (duration * SPEED_OF_SOUND) / 2.0;
}

// NOTE: fireEngine() is not used in this test sketch.

// ====================================
// DEBUG & DISPLAY FUNCTION 
// ====================================

void displayStatus(float dist_cm, bool ble_connected) {
  
  // 1. DETERMINE NEW MODE
  Mode new_mode = current_mode;
  
  if (!isArmed) {
    new_mode = MODE_DISARMED;
  } else if (ble_connected) {
    if (sensor_enabled) {
      if (dist_cm <= STAGING_THRESHOLD_CM) {
        new_mode = MODE_SENSOR_STAGED;
      } else {
        new_mode = MODE_SENSOR_IDLE; 
      }
    } else {
      new_mode = MODE_MANUAL_READY; 
    }
  } 

  // 2. DETECT SWITCH AND NOTIFY USER
  if (new_mode != current_mode) {
    current_mode = new_mode;
    
    Serial.print("--------------------------------");
    Serial.print("\nMODE SWITCHED TO: ");
    if (new_mode == MODE_DISARMED) Serial.println("DISARMED");
    else if (new_mode == MODE_SENSOR_STAGED) Serial.println("SENSOR AUTO-TRIGGER (STAGED)");
    else if (new_mode == MODE_SENSOR_IDLE) Serial.println("SENSOR AUTO-TRIGGER (IDLE)");
    else if (new_mode == MODE_MANUAL_READY) Serial.println("MANUAL TRIGGER READY");
    Serial.println("--------------------------------");

    // Transient LED Flash
    if (new_mode != MODE_DISARMED) {
        matrix.renderBitmap(SWITCH_PATTERN, 8, 12);
        delay(100); 
        matrix.clear();
    }
  }

  // 3. CONTINUOUS STATUS DISPLAY
  blink_state = (blink_state + 1) % 10; 

  // Serial Monitor: Continuous Status Print
  Serial.print("Status: ");
  Serial.print(current_mode == MODE_DISARMED ? "DISARMED" : (sensor_enabled ? "Sensor: ON" : "Sensor: OFF"));
  Serial.print(" | ");
  
  if (current_mode == MODE_SENSOR_IDLE || current_mode == MODE_SENSOR_STAGED) {
      Serial.print("Dist: ");
      if (dist_cm >= 400.0) {
          Serial.println("Out of Range");
      } else {
          Serial.print(dist_cm);
          Serial.println(" cm");
      }
  } else {
      Serial.println("Sensor: N/A");
  }

  // LED Matrix: Continuous Mode Indicator
  if (current_mode == MODE_MANUAL_READY) {
    matrix.renderBitmap(BLE_CONNECTED_PATTERN, 8, 12);
  } else if (current_mode == MODE_SENSOR_STAGED) {
    matrix.renderBitmap(STAGED_PATTERN, 8, 12);
  } else if (current_mode == MODE_SENSOR_IDLE) {
    // Sensor is active but nothing in front: show simple slow blink
    if (blink_state > 5) {
      uint8_t SENSOR_IDLE_DOT[8][12] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, 
        { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
      };
      matrix.renderBitmap(SENSOR_IDLE_DOT, 8, 12);
    } else {
      matrix.clear(); 
    }
  } else if (current_mode == MODE_DISARMED) {
      matrix.clear();
  }
}

// ====================================
// SETUP FUNCTION
// ====================================
void setup() {
  Serial.begin(9600); 
  matrix.begin();
  matrix.clear();
  
  // NOTE: fanESC.attach() is removed since Servo is not needed
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("System Initialized. Awaiting BLE Connection to Arm.");
  current_mode = MODE_DISARMED; 

  // --- BLE Setup ---
  if (!BLE.begin()) {
    Serial.println("BLE initialization failed!");
    while (1);
  }
  BLE.setLocalName("DerbyCarV5");
  BLE.setAdvertisedService(carService);
  carService.addCharacteristic(startCommandCharacteristic);
  BLE.addService(carService);
  carService.addCharacteristic(modeCharacteristic);
  BLE.addService(carService);

  // Set default mode: Manual Trigger (1)
  modeCharacteristic.writeValue(1);
  sensor_enabled = false;
  
  startCommandCharacteristic.writeValue(0); 
  BLE.advertise();
  Serial.println("BLE Advertising as DerbyCarV5...");
}

// ====================================
// LOOP FUNCTION (CLEANED UP)
// ====================================
void loop() {
  BLEDevice central = BLE.central();
  bool ble_connected = central; 
  float distance = 0.0; 

  // --- A. BLE-REQUIRED ARMING/DISARMING ---
  if (ble_connected && !isArmed) {
    Serial.println("\n--- BLE CONNECTED: ARMED ---");
    // ESC Arming commands removed for testing safety
    isArmed = true;
  } 
  
  if (!ble_connected && isArmed) {
    Serial.println("\n--- BLE DISCONNECTED: DISARMED ---");
    isArmed = false;
    current_mode = MODE_DISARMED;
  }
  
  // --- B. BLE MODE AND TRIGGER HANDLING (Only when armed) ---
  if (isArmed) {
    
    // 1. CHECK FOR MODE CHANGE COMMAND
    if (modeCharacteristic.written()) {
      int mode_cmd = modeCharacteristic.value();
      if (mode_cmd == 1) { // Manual Trigger Mode
        sensor_enabled = false;
        Serial.println("Mode changed to: MANUAL TRIGGER.");
      } else if (mode_cmd == 2) { // Sensor Auto-Trigger Mode
        sensor_enabled = true;
        Serial.println("Mode changed to: SENSOR AUTO-TRIGGER.");
      }
      modeCharacteristic.writeValue(mode_cmd); 
    }

    // 2. CHECK FOR RACE START COMMAND (For testing connectivity)
    if (startCommandCharacteristic.written()) {
      int command = startCommandCharacteristic.value();
      if (command == 1) {
        Serial.println("--- BLE MANUAL TRIGGER RECEIVED (TEST OK) ---");
        // fireEngine(); // Removed for testing
        startCommandCharacteristic.writeValue(0); 
      }
    }
  }

  // --- C. ULTRASONIC SENSOR CHECK (Only when armed AND sensor is enabled) ---
  if (isArmed && sensor_enabled) {
    distance = getDistance_cm(); 
    
    // Test the staging flag logic
    if (distance <= STAGING_THRESHOLD_CM) {
        // Staged
        // NOTE: Launch logic is removed for testing
    } 
  }
  
  // D. Display current status (Updates Serial and Matrix)
  displayStatus(distance, ble_connected); 
  
  delay(BLINK_DELAY_MS); 
}