/* * THE AUTONOMOUS LINE FOLLOWER AND OBSTACLE DETECTION CODE
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- OLED SETUP ---
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool hasOLED = false; 

// --- PIN DEFINITIONS ---
const int SENSOR_L = 2;
const int SENSOR_C = 3;
const int SENSOR_R = 4;

const int PWMA = 5;
const int AIN1 = 8;
const int AIN2 = 9;
const int PWMB = 6;
const int BIN1 = 10;
const int BIN2 = 11;
const int STBY = 7;

const int TRIG_PIN = A0;
const int ECHO_PIN = A1;
const int BUZZER_PIN = 12;

// --- TUNING & CONSTANTS ---
const int STOP_DISTANCE = 10; // Stops exactly at 10cm

float Kp = 40;       
float Kd = 250;      

int baseSpeed = 140; 
int maxSpeed = 160;  
int turnSpeed = 140; 

int lastError = 0;

// --- TIMERS & UI STATES ---
unsigned long lastDisplayTime = 0;
unsigned long lastRadarTime = 0;
int currentDistance = 999;

const char* currentDirection = "STARTING"; 
bool isSad = false; 

void setup() {
  pinMode(SENSOR_L, INPUT); pinMode(SENSOR_C, INPUT); pinMode(SENSOR_R, INPUT);
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); 

  // --- STARTUP BEEP ---
  // Confirms the buzzer has power!
  digitalWrite(BUZZER_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
  
  // --- INITIALIZE OLED SAFELY ---
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    hasOLED = true; 
    display.setRotation(2); // Flips screen 180 degrees
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 24);
    display.print(F("SYSTEM ON")); 
    display.display();
  }

  digitalWrite(STBY, HIGH); 
  
  // --- 3-SECOND COUNTDOWN ---
  for(int i = 0; i < 3; i++) { 
    digitalWrite(LED_BUILTIN, HIGH); delay(500); 
    digitalWrite(LED_BUILTIN, LOW); delay(500); 
  }
  
  // --- SOFT START ---
  for(int speed = 50; speed <= baseSpeed; speed += 10) {
    move(speed, speed);
    delay(20); 
  }
}

void loop() {
  // 1. THE RADAR TIMER (60ms between pings to prevent sensor jams)
  if (millis() - lastRadarTime >= 60) {
    currentDistance = getDistance();
    lastRadarTime = millis();
  }
  
  // 2. OBSTACLE DETECTED!
  if (currentDistance > 0 && currentDistance <= STOP_DISTANCE) {
    isSad = true; // Triggers the Sad Mouth
    currentDirection = "HALTED!";
    stopMotors();
    digitalWrite(BUZZER_PIN, HIGH); // Sounds the alarm!
  } 
  // 3. PATH IS CLEAR!
  else {
    isSad = false; // Triggers the Smile
    digitalWrite(BUZZER_PIN, LOW); // Silences the alarm
    pidLoop(); 
  }

  // 4. ANIMATION ENGINE (150ms screen refresh)
  if (millis() - lastDisplayTime >= 150) {
    drawMouth();
    lastDisplayTime = millis();
  }
}

// --- LIGHTWEIGHT OLED DRAWING FUNCTION ---
void drawMouth() {
  if (!hasOLED) return; 

  display.clearDisplay();
  
  // 1. DRAW THE PERFECTLY SIZED MOUTH
  if (isSad) {
    // Frown: Shifted down slightly to clear the HUD
    display.fillCircle(64, 28, 18, WHITE);
    display.fillCircle(64, 38, 20, BLACK);  
  } else {
    // Smile: Shifted perfectly into the middle of the screen
    display.fillCircle(64, 22, 18, WHITE);
    display.fillCircle(64, 12, 20, BLACK); 
  }

  // 2. DRAW THE DISTANCE HUD (Top Left Corner - Size 1 Font)
  display.setTextSize(1); 
  display.setCursor(0, 0); // Locked to the absolute top left
  if (currentDistance == 999) {
    display.print(F("Dist: --")); 
  } else {
    display.print(F("Dist: "));
    display.print(currentDistance);
    display.print(F("cm"));
  }

  // 3. DRAW THE DIRECTION TEXT (Bottom Center - Size 2 Font)
  display.setTextSize(2);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(currentDirection, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 45); 
  
  display.print(currentDirection);
  display.display();
}

// --- ULTRASONIC FUNCTION ---
int getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 10000); 
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

// --- PID MATH ---
void pidLoop() {
  int L = digitalRead(SENSOR_L);
  int C = digitalRead(SENSOR_C);
  int R = digitalRead(SENSOR_R);
  int error = lastError;

  if (L == 0 && C == 1 && R == 0) error = 0;       
  else if (L == 0 && C == 1 && R == 1) error = 1;  
  else if (L == 0 && C == 0 && R == 1) error = 2;  
  else if (L == 1 && C == 1 && R == 0) error = -1; 
  else if (L == 1 && C == 0 && R == 0) error = -2; 
  else if (L == 0 && C == 0 && R == 0) {           
     if (lastError > 0) error = 3;  
     else error = -3;               
  }
  else if (L == 1 && C == 1 && R == 1) error = 0; 

  // --- UPDATE THE OLED TEXT ---
  if (error == 0) currentDirection = "STRAIGHT";
  else if (error == 1 || error == 2) currentDirection = "RIGHT";
  else if (error == -1 || error == -2) currentDirection = "LEFT";
  else if (error == 3 || error == -3) currentDirection = "U TURN";

  int P = error;
  int D = error - lastError;
  int correction = (Kp * P) + (Kd * D);
  lastError = error;

  if (error == 3) {
    move(turnSpeed, -turnSpeed); 
  }
  else if (error == -3) {
    move(-turnSpeed, turnSpeed); 
  }
  else {
    int leftMotorSpeed = baseSpeed + correction;
    int rightMotorSpeed = baseSpeed - correction;

    leftMotorSpeed = constrain(leftMotorSpeed, -maxSpeed, maxSpeed);
    rightMotorSpeed = constrain(rightMotorSpeed, -maxSpeed, maxSpeed);

    move(leftMotorSpeed, rightMotorSpeed);
  }
}

// --- MOTOR CONTROLS ---
void move(int speedL, int speedR) {
  if (speedL >= 0) {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
    analogWrite(PWMA, speedL);
  } else {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, -speedL);
  }

  if (speedR >= 0) {
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    analogWrite(PWMB, speedR);
  } else {
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, -speedR);
  }
}

void stopMotors() {
  digitalWrite(PWMA, LOW); 
  digitalWrite(PWMB, LOW);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
}