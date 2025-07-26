#include <WiFi.h>
#include <HTTPClient.h>
#include "HX711.h"
#include <Update.h>
#include <ArduinoJson.h>
#include "driver/i2s.h"
#include <arduinoFFT.h>

// --- Ustawienia WiFi i API ---
const char* firmware_url = "https://pszczol.one.pl/firmware/esp32_latest.bin";
const char* config_url = "http://pszczol.one.pl/settings.json";
const char* report_url = "http://pszczol.one.pl/api/report_update.php";
const char* change_url = "http://pszczol.one.pl/api/report_change.php";
String wifi_ssid = "AirPortExtreme";
String wifi_password = "Flash255";
const char* serverName = "http://pszczol.one.pl/api/add.php";

const int BOARD_ID = 2; // zmień dla każdej płytki (1-4)
int boardId = BOARD_ID;

String prev_ssid = wifi_ssid;
String prev_password = wifi_password;
String authHeader = "Basic bGFzZXI6bGFzZXIxMjM=";

// --- HX711 (waga) ---
HX711 scale;
uint8_t dataPin = 16;
uint8_t clockPin = 4;
float previous = 0;
bool firmwareUpdateFlag = false;
int loopDelay = 10;
float offsetVal = -598696;
float scaleVal = -25.353687;

// --- INMP441 mikrofon (ESP32 I2S) ---
#define MIC_BCLK 26
#define MIC_WS   25
#define MIC_SD   33
const i2s_port_t I2S_PORT = I2S_NUM_0;
const int SAMPLE_RATE = 16000;
const int SAMPLES = 1024;  // 1024 = ok. 64 ms, FFT ~15,6 Hz rozdzielczości

int32_t micBuffer[SAMPLES];
float vReal[SAMPLES];
float vImag[SAMPLES];
ArduinoFFT<float> FFT(vReal, vImag, (uint16_t)SAMPLES, (float)SAMPLE_RATE);

// --- konfiguracja ---
bool prevFirmwareUpdate = false;
int prevLoopDelay = 10;
float prevOffsetVal = -598696;
float prevScaleVal = -25.353687;

void reportUpdate(bool success);
void reportChange();

// --------- FIRMWARE I KONFIG ---------
bool checkFirmwareUpdate() {
  WiFiClient client;
  HTTPClient https;
  Serial.println(" ");
  Serial.println("Sprawdzanie aktualizacji...");
  https.begin(client, firmware_url);

  int httpCode = https.GET();
  bool success = false;

  if (httpCode == 200) {
    int len = https.getSize();
    WiFiClient* stream = https.getStreamPtr();

    if (Update.begin(len)) {
      size_t written = Update.writeStream(*stream);

      if (written == len && Update.end()) {
        Serial.println(" ");
        Serial.println("Aktualizacja zakończona sukcesem, restartuję...");
        success = true;
      } else {
        Serial.println("Błąd aktualizacji: " + String(Update.getError()));
      }
    } else {
      Serial.println("Nie można rozpocząć aktualizacji.");
    }
  } else {
    Serial.println("Brak aktualizacji. ");
    Serial.println(" ");
  }
  https.end();
  reportUpdate(success);
  if (success) {
    ESP.restart();
  }
  return success;
}

bool fetchConfig() {
  HTTPClient http;
  http.begin(config_url);
  int httpCode = http.GET();
  bool triggerUpdate = false;
  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err && doc.containsKey("boards")) {
      JsonArray boards = doc["boards"].as<JsonArray>();
      for (JsonObject b : boards) {
        if (b["board_id"] == boardId) {
          firmwareUpdateFlag = b["firmwareUpdate"];
          loopDelay = b["loopDelay"];
          offsetVal = b["offset"];
          scaleVal = b["scale"];
          if (b.containsKey("wifi_ssid")) {
            wifi_ssid = String((const char*)b["wifi_ssid"]);
          }
          if (b.containsKey("wifi_password")) {
            wifi_password = String((const char*)b["wifi_password"]);
          }
          break;
        }
      }
    }
  }
  http.end();

  if (firmwareUpdateFlag != prevFirmwareUpdate ||
      loopDelay != prevLoopDelay ||
      offsetVal != prevOffsetVal ||
      scaleVal != prevScaleVal ||
      wifi_ssid != prev_ssid ||
      wifi_password != prev_password) {
    Serial.println("CONFIG CHANGE DETECTED");
    reportChange();
    scale.set_offset(offsetVal);
    scale.set_scale(scaleVal);
    triggerUpdate = (!prevFirmwareUpdate && firmwareUpdateFlag);
    prevFirmwareUpdate = firmwareUpdateFlag;
    prevLoopDelay = loopDelay;
    prevOffsetVal = offsetVal;
    prevScaleVal = scaleVal;
    if (wifi_ssid != prev_ssid || wifi_password != prev_password) {
      Serial.println("Reconnecting WiFi...");
      WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      Serial.println();
      prev_ssid = wifi_ssid;
      prev_password = wifi_password;
    }
  }
  return triggerUpdate;
}

void reportUpdate(bool success) {
  HTTPClient http;
  http.begin(report_url);
  http.addHeader("Content-Type", "application/json");
  String payload = String("{\"success\":") + (success ? "true" : "false") + "}";
  http.POST(payload);
  http.end();
}

void reportChange() {
  HTTPClient http;
  http.begin(change_url);
  http.addHeader("Content-Type", "application/json");
  String payload = String("{\"firmwareUpdate\":") + (firmwareUpdateFlag ? "true" : "false") +
                    ",\"loopDelay\":" + String(loopDelay) +
                    ",\"offset\":" + String(offsetVal) +
                    ",\"scale\":" + String(scaleVal) + "}";
  http.POST(payload);
  http.end();
}

// --------- I2S mikrofon ---------
void setupMicrophone() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = MIC_BCLK,
    .ws_io_num = MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MIC_SD
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

// --------- FFT: wykrycie częstotliwości ---------
float readFrequency() {
  size_t bytesRead = 0;
  // Uwaga: przekazujemy wskaźnik typu void*
  esp_err_t err = i2s_read(I2S_PORT, (void*)micBuffer, sizeof(micBuffer), &bytesRead, portMAX_DELAY);
  if (err != ESP_OK || bytesRead == 0) {
    Serial.println("i2s_read error!");
    return 0.0;
  }
  int samples = bytesRead / sizeof(int32_t);
  if (samples < SAMPLES) return 0.0;

  int64_t sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += micBuffer[i];
  float mean = (float)sum / SAMPLES;
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = (float)micBuffer[i] - mean;
    vImag[i] = 0.0;
  }

  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();

  int bin_low = ceil(5.0 * SAMPLES / SAMPLE_RATE);
  int bin_high = floor(400.0 * SAMPLES / SAMPLE_RATE);
  float peak = 0;
  int peakBin = 0;
  for (int i = bin_low; i <= bin_high; i++) {
    if (vReal[i] > peak) {
      peak = vReal[i];
      peakBin = i;
    }
  }
  float freq = (float)peakBin * SAMPLE_RATE / SAMPLES;
  return freq;
}

// --------- SETUP ---------
void setup() {
  Serial.begin(115200);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.println("Łączenie z WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nPołączono!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println(" ");

  scale.begin(dataPin, clockPin);
  scale.set_offset(offsetVal);
  scale.set_scale(scaleVal);
  scale.tare();
  setupMicrophone();

  bool doUpdateStartup = fetchConfig();
  if (doUpdateStartup) {
    checkFirmwareUpdate();
  }
}

// --------- PĘTLA GŁÓWNA ---------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Utracono połączenie z WiFi. Ponowne łączenie...");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000);
    return;
  }
  bool doUpdate = fetchConfig();

  if (doUpdate) {
    checkFirmwareUpdate();
  }

  float weight = getStableWeight();
  int32_t rssi = WiFi.RSSI();
  float hz = readFrequency();

  Serial.print("Waga: ");
  Serial.print(weight);
  Serial.print(" g, Hz: ");
  Serial.print(hz);
  Serial.print(", RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", authHeader);

  String jsonPayload = "{\"weight\":" + String(weight) + ",\"hz\":" + String(hz) + ",\"board\":" + String(boardId) + "}";

  int httpResponseCode = http.POST(jsonPayload);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  Serial.println(" ");

  http.end();
  delay(loopDelay * 1000);
}

// --------- FUNKCJA WAŻENIA ---------
float getStableWeight() {
  float w1 = scale.get_units(10);
  delay(100);
  float w2 = scale.get_units();
  while (abs(w1 - w2) > 10) {
    w1 = w2;
    w2 = scale.get_units();
    delay(100);
  }
  return w1;
}
