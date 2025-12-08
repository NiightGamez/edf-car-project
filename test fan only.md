
#include <Servo.h>

Servo testESC; // Create servo object to control ESC
const int ESC_PIN = 9; // Connect ESC signal wire to Digital Pin 9

// Standard ESC pulse values
const int THROTTLE_MIN = 1000; // 0% throttle (Arming)
const int THROTTLE_MAX = 2000; // 100% throttle

void setup() {
  Serial.begin(9600);
  testESC.attach(ESC_PIN);
  
  // --- STEP 1: ARMING SEQUENCE ---
  Serial.println("STEP 1: Disconnect USB and plug in LiPo battery NOW.");
  Serial.println("Waiting 5 seconds for ESC to arm at min throttle...");
  
  // Send the minimum pulse required to initialize the ESC
  testESC.writeMicroseconds(THROTTLE_MIN);
  
  // Wait for the ESC's internal arming sequence (listen for confirmation beep!)
  delay(5000); 
  
  Serial.println("STEP 2: ARMING COMPLETE. Starting full thrust in 3 seconds...");
  delay(3000);
}

void loop() {
  Serial.println("STEP 3: FULL THRUST ACTIVATED!");
  
  // --- STEP 3: FULL THROTTLE ---
  testESC.writeMicroseconds(THROTTLE_MAX); // Run at 100%
  delay(3000); // Run for 3 seconds
  
  // --- STEP 4: SHUTDOWN ---
  Serial.println("STEP 4: SHUTTING DOWN.");
  testESC.writeMicroseconds(THROTTLE_MIN); // Stop the motor
  
  while (true) {
    Serial.println("--- Test Complete. Disconnect battery. ---");
    delay(5000); // Wait indefinitely
  }
}