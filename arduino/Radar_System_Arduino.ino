#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -------------------- PINS --------------------
const int trigPin = 10;
const int echoPin = 11;
const int servoPin = 12;

const int greenLED = 9;
const int yellowLED = 7;
const int redLED = 8;

const int buzzerPin = 13;

// -------------------- OBJECTS --------------------
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- VARIABLES --------------------
long duration;
int distance;
unsigned long previousBeepTime = 0;

// -------------------- SETUP --------------------
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);

  myServo.attach(servoPin);
  myServo.write(15);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Radar System");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1500);
  lcd.clear();
}

// -------------------- MAIN LOOP --------------------
void loop() {
  // Scan and measure from 15 to 165 degrees.
  for (int i = 15; i <= 165; i++) {
    myServo.write(i);
    delay(30);

    distance = calculateDistance();

    // Processing reads each record until the period.
    Serial.print(i);
    Serial.print(",");
    Serial.print(distance);
    Serial.println(".");

    updateSystem(distance);
  }

  // Return to the starting angle without measuring on the return trip.
  for (int i = 165; i >= 15; i--) {
    myServo.write(i);
    delay(30);
  }
}

// -------------------- DISTANCE FUNCTION --------------------
int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999; // No echo detected.
  }

  return (int)(duration * 0.034 / 2);
}

// -------------------- SYSTEM FUNCTION --------------------
void updateSystem(int distance) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm");

  // SAFE: more than 50 cm.
  if (distance > 50) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    noTone(buzzerPin);

    lcd.setCursor(0, 1);
    lcd.print("MODE: SAFE");
  }
  // WARNING: 8–50 cm.
  else if (distance > 7 && distance <= 50) {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);

    lcd.setCursor(0, 1);
    lcd.print("MODE: WARNING");

    if (millis() - previousBeepTime >= 700) {
      previousBeepTime = millis();
      tone(buzzerPin, 1000, 120);
    }
  }
  // DANGER: 7 cm or less.
  else {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);

    lcd.setCursor(0, 1);
    lcd.print("MODE: DANGER!");

    if (millis() - previousBeepTime >= 250) {
      previousBeepTime = millis();
      tone(buzzerPin, 1500, 100);
    }
  }
}
