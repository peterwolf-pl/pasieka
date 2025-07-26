#include <ESP8266WiFi.h>
#include "core_esp8266_i2s.h"

// INMP441 connections for ESP8266
#define MIC_BCLK 14  // D5
#define MIC_WS   15  // D8
#define MIC_SD   13  // D7

#define SAMPLES 512  // Bezpiecznie dla RAM
int32_t micBuffer[SAMPLES];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Start I2S test...");

  i2s_begin();
  i2s_rxtx_begin(true, false);
  i2s_set_rate(16000); // 16 kHz

  delay(500);
}

void loop() {
  int16_t left, right;
  int samples = 0;
  int64_t sum = 0;
  int32_t minv = 32767, maxv = -32768;

  for (int i = 0; i < SAMPLES; i++) {
    if (i2s_read_sample(&left, &right, true)) {
      micBuffer[i] = left;
      sum += left;
      if (left < minv) minv = left;
      if (left > maxv) maxv = left;
      samples++;
    }
  }

  float mean = (float)sum / samples;
  float rms = 0;
  for (int i = 0; i < SAMPLES; i++) {
    float val = micBuffer[i] - mean;
    rms += val * val;
    if (i < 8) {
      Serial.print("Próbka["); Serial.print(i); Serial.print("]: "); Serial.println(val);
    }
  }
  rms = sqrt(rms / SAMPLES);

  Serial.print("Próbek: "); Serial.print(samples);
  Serial.print(" | Min: "); Serial.print(minv);
  Serial.print(" | Max: "); Serial.print(maxv);
  Serial.print(" | RMS: "); Serial.println(rms);

  delay(2000);
}
