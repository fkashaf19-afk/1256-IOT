#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- Pin Configuration ---
#define LDR_PIN 34      // Analog input for LDR
#define DHTPIN 14         // DHT data pin
#define DHTTYPE DHT11     // Change to DHT22 if using that model
#define SDA_PIN 21        // OLED SDA
#define SCL_PIN 22        // OLED SCL

// --- OLED Display Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- DHT Sensor Setup ---
DHT dht(DHTPIN, DHTTYPE);

// --- Setup Function ---
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 with LDR + DHT + OLED");

  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // Initialize DHT
  dht.begin();
  delay(1000);
}

// --- Main Loop ---
void loop() {
  // --- LDR Reading ---
  int adcValue = analogRead(LDR_PIN);
  float voltage = (adcValue / 4095.0) * 3.3;

  // --- DHT Readings ---
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // --- Check Sensor Errors ---
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error reading DHT sensor!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("DHT Error!");
    display.display();
    delay(2000);
    return;
  }

  // --- Print to Serial Monitor ---
  Serial.printf("LDR ADC: %d | Voltage: %.2f V | Temp: %.1f C | Humidity: %.1f %%\n",
                adcValue, voltage, temperature, humidity);

  // --- Display on OLED ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Sensor Readings");

  display.setCursor(0, 14);
  display.print("LDR ADC: ");
  display.println(adcValue);
  display.print("Volt: ");
  display.print(voltage, 2);
  display.println("V");

  display.setCursor(0, 34);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 48);
  display.print("Hum:  ");
  display.print(humidity, 1);
  display.println(" %");

  display.display();

  delay(2000); // Update every 2 seconds
}