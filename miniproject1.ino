#include <LiquidCrystal_I2C.h>

const int motorPin1 = 6;  // L293D IN1
const int motorPin2 = 7;  // L293D IN2
const int speedPWM = 3;   // L293D ENA (Enable PWM)
const int buzzer = 9;

const int vtPin = 4;  // VT pin from HT12D (Valid Transmission)
const int rfDataPins[4] = {A0, A1, A2, A3};  // 433MHz Receiver Data Inputs
const int speeds[4] = {20, 30, 40, 50};  // Speed limits for school zones

const int speedUpButton = 11;
const int speedDownButton = 10;

int speed = 0;  // Start at 0 speed
int schoolSpeedLimit = 255;  // Default to max speed
bool schoolZone = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(speedPWM, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(vtPin, INPUT);  // VT pin as input

    // Set RF receiver data pins as input
    for (int i = 0; i < 4; i++) {
        pinMode(rfDataPins[i], INPUT);
    }

    // Set buttons as input
    pinMode(speedUpButton, INPUT_PULLUP);
    pinMode(speedDownButton, INPUT_PULLUP);

    lcd.init();
    lcd.backlight();

    // Startup messages
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Speed Control");
    lcd.setCursor(0, 1);
    lcd.print("Device");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loading...");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome!");
    lcd.setCursor(0, 1);
    lcd.print("Drive Carefully");
    delay(2000);

    Serial.begin(9600);
    Serial.println("System Ready.");
}

void loop() {
    schoolZone = false;  // Reset school zone flag
    bool rfSignalDetected = false;
    
    if (digitalRead(vtPin) == HIGH) {  // If in school zone (VT HIGH)
        for (int i = 0; i < 4; i++) {
            if (digitalRead(rfDataPins[i]) == HIGH) {
                schoolSpeedLimit = speeds[i];  // Set speed limit based on RF data
                speed = min(speed, schoolSpeedLimit);  // Ensure speed does not exceed limit
                schoolZone = true;
                rfSignalDetected = true;
                break;
            }
        }
    } else {
        schoolSpeedLimit = 255;  // No school zone, allow max speed
    }

    // Manual control when no RF signal detected (Even if VT is HIGH)
    if (!rfSignalDetected || digitalRead(vtPin) == LOW) {
        if (digitalRead(speedUpButton) == LOW) {
            speed = min(speed + 10, 255);  // Increase speed
            delay(200);  // Debounce delay
        }
        if (digitalRead(speedDownButton) == LOW) {
            speed = max(speed - 10, 0);  // Decrease speed
            delay(200);  // Debounce delay
        }
    } else if (schoolZone) {
        // If in school zone and RF data is detected:
        if (digitalRead(speedDownButton) == LOW) {
            speed = max(speed - 10, 0);  // Allow decreasing speed
            delay(200);  // Debounce delay
        }
        if (digitalRead(speedUpButton) == LOW) {
            speed = min(speed, schoolSpeedLimit);  // Prevent increasing speed beyond school limit
            delay(200);  // Debounce delay
        }
    }

    // **Motor Control**
    if (speed > 0) {
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, LOW);   // Motor moves forward
    } else {
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);   // Stop motor
    }

    // Control motor speed using PWM
    analogWrite(speedPWM, map(speed, 0, 255, 0, 255));

    // Display speed on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Speed: ");
    lcd.print(speed);
    lcd.print(" km/h");

    if (schoolZone) {
        lcd.setCursor(0, 1);
        lcd.print("SCHOOL ZONE!");
        digitalWrite(buzzer, HIGH);
        delay(500);
        digitalWrite(buzzer, LOW);
    } else {
        lcd.setCursor(0, 1);
        lcd.print("Normal Mode");
    }

    Serial.print("Speed Set to: ");
    Serial.print(speed);
    Serial.println(" km/h");

    delay(200);  // Small delay for button responsiveness
}
