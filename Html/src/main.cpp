

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Pins ----------------
#define DHTPIN 23
#define DHTTYPE DHT22
#define BUTTON_PIN 5   // Button to GND, use INPUT_PULLUP

DHT dht(DHTPIN, DHTTYPE);

// ---------------- Wi-Fi Credentials ----------------
const char* ssid     = "Wokwi-Guest";        // <-- Replace with your Wi-Fi SSID
const char* password = " ";    // <-- Replace with your Wi-Fi password

// ---------------- Web Server ----------------
WebServer server(80);

// ---------------- Globals ----------------
float lastTemp = NAN;
float lastHum  = NAN;
bool lastButtonState = HIGH;

// ---------------- Functions ----------------

// Read DHT22 sensor
void readDHTValues() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius

  if (!isnan(h) && !isnan(t)) {
    lastHum  = h;
    lastTemp = t;
    Serial.print("Temp: "); Serial.print(t); Serial.print(" °C, Hum: "); Serial.println(h);
  } else {
    Serial.println("Failed to read from DHT!");
  }
}

// Display on OLED
void showOnOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (isnan(lastTemp) || isnan(lastHum)) {
    display.setCursor(0, 0);
    display.println("DHT Error!");
  } else {
    display.setCursor(0, 0);
    display.println("DHT22 Readings");

    display.setCursor(0, 20);
    display.print("Temp: "); display.print(lastTemp, 1); display.println(" C");

    display.setCursor(0, 40);
    display.print("Hum: "); display.print(lastHum, 1); display.println(" %");
  }

  display.display();
}

// Dynamic HTML webpage
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>"; // auto-refresh every 5 sec
  html += "<title>ESP32 DHT Monitor</title>";
  html += "<style>body { font-family: Arial; text-align:center; margin-top:20px; }";
  html += "h2 { color: #2E86C1; } p { font-size: 18px; } hr { margin:20px 0; }</style>";
  html += "</head><body>";
  html += "<h2>ESP32 DHT22 Sensor Monitor</h2>";

  if (isnan(lastTemp) || isnan(lastHum)) {
    html += "<p><b>No valid data yet.</b><br>Press the button to take a reading.</p>";
  } else {
    html += "<p><b>Temperature:</b> " + String(lastTemp, 1) + " &deg;C</p>";
    html += "<p><b>Humidity:</b> " + String(lastHum, 1) + " %</p>";
  }

  html += "<hr>";
  html += "<p><b>Device IP:</b> " + WiFi.localIP().toString() + "</p>";
  html += "<p>Press the physical button to update readings on OLED and this page.</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(0,0); display.println("Booting..."); display.display();

  dht.begin();

  // Wi-Fi connect
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("ESP32 IP Address: "); Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0,0); display.println("WiFi Connected");
  display.setCursor(0,16); display.print("IP: "); display.println(WiFi.localIP());
  display.setCursor(0,32); display.println("Press button");
  display.setCursor(0,44); display.println("to read DHT");
  display.display();

  // Web server
  server.on("/", handleRoot);
  server.begin();
}

// ---------------- Loop ----------------
void loop() {
  server.handleClient();

  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Detect falling edge (HIGH -> LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    delay(50); // debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("Button pressed: reading DHT + updating OLED");
      readDHTValues();
      showOnOLED();
    }
  }
  lastButtonState = currentButtonState;
}
