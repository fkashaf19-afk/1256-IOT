#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- OLED Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Pin Definitions ---
#define BUTTON 12
#define LED 26
#define BUZZER 33

// --- Variables ---
unsigned long pressStart = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 50ms debounce
bool buttonState = HIGH;
bool lastButtonState = HIGH;
bool ledState = LOW;
bool buttonPressed = false;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP); // Button active LOW
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // --- OLED setup ---
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Task B: Button Test");
  display.display();
}

void loop() {
  int reading = digitalRead(BUTTON);

  // --- Debounce handling ---
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) { // Button pressed
        pressStart = millis();
      }

      if (buttonState == HIGH) { // Button released
        unsigned long pressDuration = millis() - pressStart;
        display.clearDisplay();
        display.setCursor(0, 0);

        if (pressDuration < 1500) {
          // --- Short Press ---
          ledState = !ledState;
          digitalWrite(LED, ledState);
          display.println("Short Press");
          display.println(ledState ? "LED ON" : "LED OFF");
        } else {
          // --- Long Press ---
          display.println("Long Press");
          display.println("Buzzer ON");
          tone(BUZZER, 1000, 500); // 1kHz for 0.5s
        }
        display.display();
      }
    }
  }

  lastButtonState = reading;
}
