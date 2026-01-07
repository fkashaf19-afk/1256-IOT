// ----- DHT22 Sensor with OLED Display and Button -----
// This program reads temperature and humidity from a DHT22 sensor

// ----- Required Libraries -----
#include <Wire.h>                  // I2C communication library
#include <Adafruit_GFX.h>          // Graphics library for OLED
#include <Adafruit_SSD1306.h>      // OLED display driver
#include "DHT.h"                   // DHT sensor library

// ----- OLED Display Size -----
#define SCREEN_WIDTH 128            // OLED width in pixels
#define SCREEN_HEIGHT 64            // OLED height in pixels

// Create OLED display object (I2C, no reset pin = -1)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ----- Pin Definitions -----
#define DHTPIN 23                   // DHT22 data pin connected to GPIO 23
#define DHTTYPE DHT22               // Define DHT sensor type
#define BUTTON_PIN 5                // Push button connected to GPIO 5 (to GND)

// Create DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);             // Start serial communication

  // Button uses internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // ----- OLED Initialization -----
  // Address 0x3C is common for SSD1306 OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");  // Error if OLED not detected
    while (1);                          // Stop program
  }

  // Clear OLED buffer
  display.clearDisplay();

  // Set text properties
  display.setTextSize(1);            // Normal text size
  display.setTextColor(WHITE);       // White text
  display.setCursor(0, 0);           // Top-left corner

  // Show startup message
  display.println("System Ready...");
  display.display();                 // Send buffer to OLED

  // Start DHT sensor
  dht.begin();
}

void loop() {

  // Check if button is pressed (LOW because of INPUT_PULLUP)
  if (digitalRead(BUTTON_PIN) == LOW) {

    // Show reading message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Reading DHT...");
    display.display();

    delay(300);   // Button debounce delay

    // Read humidity and temperature
    float h = dht.readHumidity();        // Read humidity (%)
    float t = dht.readTemperature();     // Read temperature (Celsius)

    display.clearDisplay();

    // Check if sensor reading failed
    if (isnan(t) || isnan(h)) {
      display.setCursor(0, 0);
      display.println("DHT Error!");     // Sensor error message
    } else {
      // Display temperature & humidity
      display.setCursor(0, 0);
      display.println("DHT22 Readings");

      display.setCursor(0, 20);
      display.print("Temp: ");
      display.print(t);
      display.println(" C");

      display.setCursor(0, 40);
      display.print("Hum:  ");
      display.print(h);
      display.println(" %");
    }

    display.display();   // Update OLED
    delay(700);          // Wait before next button press
  }
}
