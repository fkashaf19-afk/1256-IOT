/*****************************************************
 * ESP32 Web Server – Temperature & Humidity Monitor
 * Sensor  : DHT11 / DHT22
 * Author  : Your Name
 * Purpose : Show sensor data on a web page
 *****************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// ---------- WiFi Credentials ----------
const char* wifiName = "Wokwi-GUEST";
const char* wifiPass = "";

// ---------- DHT Sensor Settings ----------
#define SENSOR_PIN 23
#define SENSOR_TYPE DHT11   // change to DHT22 if needed

DHT envSensor(SENSOR_PIN, SENSOR_TYPE);

// ---------- Web Server ----------
WebServer espServer(80);

// ---------- Function Prototypes ----------
void showHomePage();
String buildHTML(float t, float h);

// ---------- Create Web Page ----------
String buildHTML(float t, float h) {
  String page = "<!DOCTYPE html>";
  page += "<html><head><title>ESP32 Environment</title>";
  page += "<meta http-equiv='refresh' content='6'>";
  page += "<style>";
  page += "body{font-family:Arial;background:#f2f2f2;text-align:center;}";
  page += "div{background:#fff;padding:20px;margin:40px;border-radius:10px;}";
  page += "</style></head><body>";
  page += "<div>";
  page += "<h2>ESP32 Environmental Monitor</h2>";
  page += "<p><b>Temperature:</b> " + String(t,1) + " °C</p>";
  page += "<p><b>Humidity:</b> " + String(h,1) + " %</p>";
  page += "<p>Auto refresh every 6 seconds</p>";
  page += "</div></body></html>";
  return page;
}

// ---------- Handle Root URL ----------
void showHomePage() {
  float humidity = envSensor.readHumidity();
  float temperature = envSensor.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    espServer.send(200, "text/html",
                   "<h3>Sensor error! Please refresh.</h3>");
    return;
  }

  espServer.send(200, "text/html",
                 buildHTML(temperature, humidity));
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Web Server Starting...");

  envSensor.begin();

  WiFi.begin(wifiName, wifiPass);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected Successfully");
  Serial.print("Device IP Address: ");
  Serial.println(WiFi.localIP());

  espServer.on("/", showHomePage);
  espServer.begin();
  Serial.println("HTTP Server Started");
}

// ---------- Loop ----------
void loop() {
  espServer.handleClient();
}