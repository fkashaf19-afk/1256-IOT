#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define BUTTON1 12
#define BUTTON2 14
#define LED1 25
#define LED2 26

int mode = 0;
unsigned long prevMillis = 0;
unsigned long fadePrevMillis = 0;
int blinkState = LOW;
int pwmValue = 0;
int fadeAmount = 5;
const int fadeInterval = 20;

volatile bool btn1Pressed = false;
volatile bool btn2Pressed = false;
unsigned long lastBtn1 = 0;
unsigned long lastBtn2 = 0;

void IRAM_ATTR handleButton1() {
  unsigned long now = millis();
  if (now - lastBtn1 > 200) {
    btn1Pressed = true;
    lastBtn1 = now;
  }
}

void IRAM_ATTR handleButton2() {
  unsigned long now = millis();
  if (now - lastBtn2 > 200) {
    btn2Pressed = true;
    lastBtn2 = now;
  }
}

void showOLED(const String &text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Mode:");
  display.println(text);
  display.display();
}

void setup() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  showOLED("Both OFF");

  attachInterrupt(digitalPinToInterrupt(BUTTON1), handleButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), handleButton2, FALLING);
}

void loop() {
  // --- Handle Button 1 ---
  if (btn1Pressed) {
    btn1Pressed = false;
    mode = (mode + 1) % 4;
  }

  // --- Handle Button 2 (Reset) ---
  if (btn2Pressed) {
    btn2Pressed = false;
    mode = 0;
  }

  // --- LED Behavior ---
  switch (mode) {
    case 0: // Both OFF
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      showOLED("Both OFF");
      break;

    case 1: // Alternate Blink
      if (millis() - prevMillis >= 500) {
        prevMillis = millis();
        blinkState = !blinkState;
        digitalWrite(LED1, blinkState);
        digitalWrite(LED2, !blinkState);
      }
      showOLED("Alternate Blink");
      break;

    case 2: // Both ON
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      showOLED("Both ON");
      break;

    case 3: // PWM Fade
      if (millis() - fadePrevMillis >= fadeInterval) {
        fadePrevMillis = millis();
        analogWrite(LED1, pwmValue);
        analogWrite(LED2, pwmValue);
        pwmValue += fadeAmount;
        if (pwmValue <= 0 || pwmValue >= 255) fadeAmount = -fadeAmount;
      }
      showOLED("PWM Fade");
      break;
  }

  delay(50); // Small delay to stabilize display updates
}
