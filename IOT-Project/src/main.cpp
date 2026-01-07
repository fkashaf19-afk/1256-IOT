#include <Arduino.h>
#include <WiFi.h>

/************ BLYNK CONFIG ************/
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Qn-korQc"
#define BLYNK_TEMPLATE_NAME "smoke gas detector"
#define BLYNK_AUTH_TOKEN "MxA1hq2ju3z0V5hHu1fpJdn5swegF-ko"

#include <BlynkSimpleEsp32.h>

/************ MQTT LIBRARY ************/
#include <PubSubClient.h>

/************ LIBRARIES ************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

/************ WIFI CONFIG ************/
char ssid[] = ".....";
char pass[] = "password56";

/************ MQTT CONFIG ************/
const char* mqtt_server = "broker.hivemq.com";
WiFiClient espClient;
PubSubClient client(espClient);

/************ MQTT TOPICS ************/
#define TOPIC_GAS     "esp32/smoke/gas"
#define TOPIC_TEMP    "esp32/smoke/temp"
#define TOPIC_HUM     "esp32/smoke/humidity"
#define TOPIC_STATUS  "esp32/smoke/status"

/************ PIN CONFIG ************/
#define MQ2_PIN     34
#define BUZZER_PIN  26
#define LED_PIN     25

#define DHT_PIN     27
#define DHT_TYPE    DHT11

/************ OLED CONFIG ************/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define OLED_ADDR    0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/************ OBJECTS ************/
DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

/************ VARIABLES ************/
int gasThreshold = 1400;
int baseline = 0;

bool ledState = false;
bool buzzerState = false;

float currentTemp = 0.0;
float currentHumidity = 0.0;

/************ MQTT CONNECT ************/
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_SmokeDetector")) {
      Serial.println("connected");
    } else {
      Serial.print("failed rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

/************ BASELINE CALIBRATION ************/
void calibrateBaseline() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Calibrating MQ2...");
  display.display();

  long sum = 0;
  const int samples = 50;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(MQ2_PIN);
    delay(100);
  }

  baseline = sum / samples;

  display.clearDisplay();
  display.println("Calibration Done");
  display.display();
  delay(1000);
}

/************ READ MQ2 ************/
void readMQ2() {
  int rawValue = analogRead(MQ2_PIN);
  int gasValue = rawValue - baseline;
  if (gasValue < 0) gasValue = 0;

  bool danger = gasValue > gasThreshold;

  if (danger) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    ledState = true;
    buzzerState = true;
    Blynk.logEvent("smoke_alert", "Smoke/Gas Detected!");
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    ledState = false;
    buzzerState = false;
  }

  // Blynk
  Blynk.virtualWrite(V0, gasValue);
  Blynk.virtualWrite(V1, ledState);
  Blynk.virtualWrite(V2, buzzerState);

  // MQTT
  if (!client.connected()) reconnectMQTT();
  client.loop();
  client.publish(TOPIC_GAS, String(gasValue).c_str());
  client.publish(TOPIC_STATUS, danger ? "DANGER" : "SAFE");

  // OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Smoke & Gas Monitor");

  display.setCursor(0, 16);
  display.print("Gas: ");
  display.println(gasValue);

  display.setCursor(0, 28);
  display.print("Temp: ");
  display.print(currentTemp);
  display.println(" C");

  display.setCursor(0, 40);
  display.print("Hum: ");
  display.print(currentHumidity);
  display.println(" %");

  display.setCursor(0, 52);
  display.print("Status: ");
  display.println(danger ? "DANGER" : "SAFE");

  display.display();
}

/************ READ DHT ************/
void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read failed");
    return;
  }

  currentTemp = t;
  currentHumidity = h;

  Blynk.virtualWrite(V4, currentTemp);
  Blynk.virtualWrite(V5, currentHumidity);

  if (!client.connected()) reconnectMQTT();
  client.loop();
  client.publish(TOPIC_TEMP, String(currentTemp).c_str());
  client.publish(TOPIC_HUM, String(currentHumidity).c_str());
}

/************ BLYNK CONTROLS ************/
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

BLYNK_WRITE(V2) {
  buzzerState = param.asInt();
  digitalWrite(BUZZER_PIN, buzzerState);
}

/************ SETUP ************/
void setup() {
  Serial.begin(9600);

  pinMode(MQ2_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.println("System Starting...");
  display.display();

  dht.begin();
  calibrateBaseline();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  client.setServer(mqtt_server, 1883);

  timer.setInterval(1000L, readMQ2);
  timer.setInterval(2000L, readDHT);
}

/************ LOOP ************/
void loop() {
  Blynk.run();
  timer.run();
  client.loop();
}
