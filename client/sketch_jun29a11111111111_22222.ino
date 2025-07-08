#include <WiFi.h>
#include <HTTPClient.h>
#include "HX711.h"
#include <Update.h>
#include <ArduinoJson.h>

const char* firmware_url = "https://pszczol.one.pl/firmware/esp32_latest.bin";
const char* config_url = "http://pszczol.one.pl/settings.json";
const char* report_url = "http://pszczol.one.pl/api/report_update.php";
const char* change_url = "http://pszczol.one.pl/api/report_change.php";
String wifi_ssid = "AirPortExtreme";
String wifi_password = "Flash255";
const char* serverName = "http://pszczol.one.pl/api/add.php";

const int BOARD_ID = 1; // change for each device (1-4)
int boardId = BOARD_ID;
String prev_ssid = wifi_ssid;
String prev_password = wifi_password;
String authHeader = "Basic bGFzZXI6bGFzZXIxMjM=";

HX711 scale;
uint8_t dataPin = 16;
uint8_t clockPin = 4;
float previous = 0;
bool firmwareUpdateFlag = false;
int loopDelay = 10;
float offsetVal = -598696;
float scaleVal = -25.353687;

bool prevFirmwareUpdate = false;
int prevLoopDelay = 10;
float prevOffsetVal = -598696;
float prevScaleVal = -25.353687;

void reportUpdate(bool success);
void reportChange();

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

  bool doUpdateStartup = fetchConfig();
  if (doUpdateStartup) {
    checkFirmwareUpdate();
  }
}

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

  Serial.print("Waga: ");
  Serial.print(weight);
  Serial.print(" g, RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", authHeader);

  String jsonPayload = "{\"weight\":" + String(weight) + ",\"board\":" + String(boardId) + "}";

 int httpResponseCode = http.POST(jsonPayload);
//
 Serial.print("HTTP Response code: ");
 Serial.println(httpResponseCode);
 Serial.println(" ");
//  Serial.println(" ");
//  https.end();
 delay(600);
  http.end();
  delay(loopDelay * 1000);
}

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
