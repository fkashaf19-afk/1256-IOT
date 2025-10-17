#include <Arduino.h>

// --- LED setup ---
#define LED_PIN     18
#define LED_CH      0
#define LED_FREQ    5000
#define LED_RES     8

// --- Buzzer setup ---
#define BUZZER_PIN  27
#define BUZZER_CH   1
#define BUZZER_FREQ 2000
#define BUZZER_RES  10

void setup() {
  // --- LED PWM setup ---
  ledcSetup(LED_CH, LED_FREQ, LED_RES);
  ledcAttachPin(LED_PIN, LED_CH);

  // --- Buzzer PWM setup ---
  ledcSetup(BUZZER_CH, BUZZER_FREQ, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);

  // --- 1. Simple beep pattern ---
  for (int i = 0; i < 3; i++) {
    ledcWriteTone(BUZZER_CH, 2000 + i * 400); // Beep tone
    delay(150);
    ledcWrite(BUZZER_CH, 0);                  // Stop tone
    delay(150);
  }

  // --- 2. Frequency sweep (400Hz → 3kHz) ---
  for (int f = 400; f <= 3000; f += 100) {
    ledcWriteTone(BUZZER_CH, f);
    delay(20);
  }
  ledcWrite(BUZZER_CH, 0);
  delay(500);

  // --- 3. Short melody ---
  int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
  for (int i = 0; i < 8; i++) {
    ledcWriteTone(BUZZER_CH, melody[i]);
    delay(250);
  }
  ledcWrite(BUZZER_CH, 0); // stop buzzer
}

void loop() {
  // --- LED brightness fade ---
  for (int d = 0; d <= 255; d++) {
    ledcWrite(LED_CH, d);
    delay(10);
  }
  for (int d = 255; d >= 0; d--) {
    ledcWrite(LED_CH, d);
    delay(10);
  }
}