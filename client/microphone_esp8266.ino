#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "core_esp8266_i2s.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";
const char* serverName = "http://pszczol.one.pl/api/add.php";

// INMP441 connections for ESP8266
// VCC -> 3.3V
// GND -> GND
// SCK -> GPIO14 (D5)
// WS  -> GPIO15 (D8)
// SD  -> GPIO13 (D7)
#define MIC_BCLK 14
#define MIC_WS   15
#define MIC_SD   13

const int SAMPLE_RATE = 16000;
const int SAMPLES = 1024;
int32_t micBuffer[SAMPLES];

void setupMic() {
  // Initialize I2S and set pin mapping before starting reception
  i2s_begin();
  i2s_set_pin(MIC_BCLK, MIC_WS, MIC_SD);
  // Enable I2S reception only and configure the sample rate
  i2s_rxtx_begin(true, false);
  i2s_set_rate(SAMPLE_RATE);
}

int readSamples() {
  int16_t left, right;
  int samples = 0;
  while (samples < SAMPLES && i2s_read_sample(&left, &right, true)) {
    micBuffer[samples++] = left;
  }
  return samples;
}

float readFrequency() {
  int samples = readSamples();
  int crossings = 0;
  for (int i = 1; i < samples; i++) {
    if ((micBuffer[i - 1] < 0 && micBuffer[i] >= 0) ||
        (micBuffer[i - 1] > 0 && micBuffer[i] <= 0)) {
      crossings++;
    }
  }
  return (crossings / 2.0f) * ((float)SAMPLE_RATE / samples);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  setupMic();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float hz = readFrequency();
    Serial.printf("Hz: %.2f\n", hz);

    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    String payload = String("{\"hz\":") + String(hz, 2) + "}";
    http.POST(payload);
    http.end();
  }
  delay(5000);
}

