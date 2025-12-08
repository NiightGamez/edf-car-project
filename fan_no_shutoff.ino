#include <Servo.h>

  

// Initialize ESC Object

Servo fanESC;

  

// --- Pin Definitions ---

const int ESC_PIN = 9;

const int TRIG_PIN = 5;

const int ECHO_PIN = 6;

  

// --- ESC & Race Constants ---

const int THROTTLE_MIN = 1000; // 0% throttle (Arming and Stopping)

const int THROTTLE_MAX = 2000; // 100% throttle

const float SPEED_OF_SOUND = 0.0343; // cm/microsecond

const int STAGING_THRESHOLD_CM = 10; // Distance to be considered 'staged' (e.g., pin is there)

const int RACE_DURATION_MS = 3000;

  

// --- State Variables ---

bool isArmed = false; // True after ESC arming sequence completes

bool isRacing = false; // True during the 3-second motor run

bool isStaged = false; // True if the starting pin is currently in front of the sensor

  

// ====================================

// CORE FUNCTION: MEASURE DISTANCE

// ====================================

float getDistance_cm() {

// Clear the trigger pin

digitalWrite(TRIG_PIN, LOW);

delayMicroseconds(2);

// Send a 10µs pulse to trigger

digitalWrite(TRIG_PIN, HIGH);

delayMicroseconds(10);

digitalWrite(TRIG_PIN, LOW);

// Measure the duration of the incoming pulse

long duration = pulseIn(ECHO_PIN, HIGH, 30000);

if (duration == 0) return 400.0; // Return high value if no echo within 30ms

// Calculate distance: (Duration * Speed of Sound) / 2

return (duration * SPEED_OF_SOUND) / 2.0;

}

  

// ====================================

// CORE FUNCTION: ACTIVATE FAN

// ====================================

void fireEngine() {

if (!isRacing && isArmed) {

Serial.println("!!! RACE START: FULL THRUST ACTIVATED !!!");

isRacing = true;

isStaged = false; // Reset staging flag

  

// --- ACTIVATE MOTOR ---

fanESC.writeMicroseconds(THROTTLE_MAX); // Run fan at 100%

Serial.print("Running for ");

Serial.print(RACE_DURATION_MS / 1000);

Serial.println(" seconds...");

delay(RACE_DURATION_MS); // Hold throttle for race duration

// --- STOP MOTOR ---

fanESC.writeMicroseconds(THROTTLE_MIN); // Stop the motor

isRacing = false;

isArmed = false; // Disarm until reset or power cycle

Serial.println("RACE ENDED. System is DISARMED.");

}

}

  

// ====================================

// SETUP FUNCTION

// ====================================

void setup() {

Serial.begin(9600);

fanESC.attach(ESC_PIN);

pinMode(TRIG_PIN, OUTPUT);

pinMode(ECHO_PIN, INPUT);

// --- ESC ARMING SEQUENCE (Copied from your working code) ---

Serial.println("--- ESC ARMING START ---");

Serial.println("STEP 1: Plug in LiPo battery NOW.");

Serial.println("Waiting 5 seconds for ESC to arm at min throttle...");

fanESC.writeMicroseconds(THROTTLE_MIN); // Send the minimum pulse

delay(5000);

isArmed = true;

Serial.println("STEP 2: ARMING COMPLETE. System is ready to stage.");

Serial.println("--------------------------------");

}

  

// ====================================

// LOOP FUNCTION (SENSOR LOGIC)

// ====================================

void loop() {

// Only check sensor if the system is armed and not already racing

if (isArmed && !isRacing) {

float distance = getDistance_cm();

// 1. Update staging memory

if (distance <= STAGING_THRESHOLD_CM) {

if (!isStaged) {

Serial.println("STATUS: STAGED. Waiting for Pin Drop.");

}

isStaged = true; // Pin is in place

}

// 2. RACE TRIGGER CHECK: Launch if car was staged AND distance increases

if (isStaged && distance > STAGING_THRESHOLD_CM) {

Serial.println("TRIGGER: Pin dropped (Distance > 10cm).");

fireEngine();

}

}

// If system is disarmed after a race, keep motor off

if (!isArmed) {

fanESC.writeMicroseconds(THROTTLE_MIN);

}

delay(50); // Small delay for loop stability

}