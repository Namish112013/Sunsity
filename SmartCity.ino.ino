// AI for a Better Future — basic sensor/automation demonstrator
// Target: Arduino Uno-compatible board
// Example pin map; verify against your actual modules before wiring.
 
const byte LDR_PIN = A0;
const byte PIR_PIN = 2;
const byte TRIG_PIN = 3;
const byte ECHO_PIN = 4;
 
const byte RED_LED = 5;
const byte YELLOW_LED = 6;
const byte GREEN_LED = 7;
const byte STREET_LED = 8;
const byte ALERT_LED = 9;
 
const int DARK_THRESHOLD = 450;  // Calibrate for your LDR divider
const unsigned long TRAFFIC_STEP_MS = 3000;
const unsigned long PULSE_TIMEOUT_US = 25000;
 
unsigned long lastTrafficChange = 0;
byte trafficState = 0;
 
void setTraffic(byte state) {
  digitalWrite(RED_LED, state == 0 ? HIGH : LOW);
  digitalWrite(GREEN_LED, state == 1 ? HIGH : LOW);
  digitalWrite(YELLOW_LED, state == 2 ? HIGH : LOW);
}
 
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
 
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  if (duration == 0) return -1.0; // No echo / out of range
  return duration * 0.0343f / 2.0f;
}
 
void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
 
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(STREET_LED, OUTPUT);
  pinMode(ALERT_LED, OUTPUT);
 
  Serial.begin(9600);
  setTraffic(0);
  lastTrafficChange = millis();
}
 
void loop() {
  // LDR: if your divider is reversed, invert this comparison.
  int lightValue = analogRead(LDR_PIN);
  digitalWrite(STREET_LED, lightValue < DARK_THRESHOLD ? HIGH : LOW);
 
  // PIR motion alert
  bool motionDetected = digitalRead(PIR_PIN) == HIGH;
  digitalWrite(ALERT_LED, motionDetected ? HIGH : LOW);
 
  // Proximity measurement
  float distanceCm = readDistanceCm();
 
  // Simple educational traffic sequence: red -> green -> yellow -> repeat.
  unsigned long now = millis();
  if (now - lastTrafficChange >= TRAFFIC_STEP_MS) {
    trafficState = (trafficState + 1) % 3;
    setTraffic(trafficState);
    lastTrafficChange = now;
  }
 
  Serial.print("LDR=");
  Serial.print(lightValue);
  Serial.print(" Motion=");
  Serial.print(motionDetected ? "YES" : "NO");
  Serial.print(" Distance_cm=");
  if (distanceCm < 0) Serial.println("NO_ECHO");
  else Serial.println(distanceCm, 1);
 
  delay(100); // Short sampling pause; not suitable for safety-critical control.
}
