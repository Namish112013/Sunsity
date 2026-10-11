// Smart City Blueprint Demonstrator
// Board: Arduino Uno-compatible
// Purpose: classroom model for streetlight automation, traffic LEDs,
// PIR security alert, ultrasonic proximity, emergency-priority demo,
// and optional analog temperature reading.

const byte LDR_PIN = A0;
const byte TEMP_PIN = A1;       // LM35/TMP36-style analog sensor demo
const byte PIR_PIN = 2;
const byte TRIG_PIN = 3;
const byte ECHO_PIN = 4;

const byte RED_LED = 5;
const byte YELLOW_LED = 6;
const byte GREEN_LED = 7;
const byte STREET_LED = 8;
const byte ALERT_LED = 9;
const byte EMERGENCY_BUTTON = 10; // Button to GND, uses INPUT_PULLUP
const byte BUZZER_PIN = 11;       // Optional active buzzer through driver if needed

const int DARK_THRESHOLD = 450;       // Calibrate using Serial Monitor
const int NEAR_DISTANCE_CM = 15;      // Proximity demo threshold
const unsigned long NORMAL_STEP_MS = 3000;
const unsigned long EMERGENCY_GREEN_MS = 5000;
const unsigned long PULSE_TIMEOUT_US = 25000;

unsigned long lastTrafficChange = 0;
unsigned long emergencyStarted = 0;
byte trafficState = 0;       // 0 red, 1 green, 2 yellow
bool emergencyMode = false;

void setTraffic(bool redOn, bool yellowOn, bool greenOn) {
  digitalWrite(RED_LED, redOn ? HIGH : LOW);
  digitalWrite(YELLOW_LED, yellowOn ? HIGH : LOW);
  digitalWrite(GREEN_LED, greenOn ? HIGH : LOW);
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  if (duration == 0) return -1.0;
  return duration * 0.0343 / 2.0;
}

float readTemperatureC() {
  int raw = analogRead(TEMP_PIN);
  float volts = raw * (5.0 / 1023.0);
  // LM35: 10 mV per degree C. For TMP36 use: (volts - 0.5) * 100.0
  return volts * 100.0;
}

void runNormalTraffic() {
  unsigned long now = millis();
  if (now - lastTrafficChange >= NORMAL_STEP_MS) {
    trafficState = (trafficState + 1) % 3;
    lastTrafficChange = now;
  }

  if (trafficState == 0) setTraffic(true, false, false);
  if (trafficState == 1) setTraffic(false, false, true);
  if (trafficState == 2) setTraffic(false, true, false);
}

void runEmergencyTraffic() {
  if (!emergencyMode) {
    emergencyMode = true;
    emergencyStarted = millis();
    setTraffic(false, false, true);
  }

  if (millis() - emergencyStarted >= EMERGENCY_GREEN_MS) {
    emergencyMode = false;
    trafficState = 2;
    lastTrafficChange = millis();
    setTraffic(false, true, false);
  }
}

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(EMERGENCY_BUTTON, INPUT_PULLUP);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(STREET_LED, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(9600);
  setTraffic(true, false, false);
  lastTrafficChange = millis();
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  bool dark = lightValue < DARK_THRESHOLD; // Reverse if your divider is opposite.
  digitalWrite(STREET_LED, dark ? HIGH : LOW);

  bool motionDetected = digitalRead(PIR_PIN) == HIGH;
  digitalWrite(ALERT_LED, motionDetected ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, motionDetected ? HIGH : LOW);

  float distanceCm = readDistanceCm();
  bool emergencyRequested = digitalRead(EMERGENCY_BUTTON) == LOW;

  if (emergencyRequested || emergencyMode) runEmergencyTraffic();
  else runNormalTraffic();

  Serial.print("LDR=");
  Serial.print(lightValue);
  Serial.print(" Dark=");
  Serial.print(dark ? "YES" : "NO");
  Serial.print(" Motion=");
  Serial.print(motionDetected ? "YES" : "NO");
  Serial.print(" Distance_cm=");
  if (distanceCm < 0) Serial.print("NO_ECHO");
  else Serial.print(distanceCm, 1);
  Serial.print(" Near=");
  Serial.print((distanceCm > 0 && distanceCm <= NEAR_DISTANCE_CM) ? "YES" : "NO");
  Serial.print(" Temp_C=");
  Serial.println(readTemperatureC(), 1);

  delay(100);
}
