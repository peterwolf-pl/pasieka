#include <WiFi.h>
#include <HTTPClient.h>
#include "HX711.h"
#include <Update.h>
#include <ArduinoJson.h>

const char* firmware_url = "http://pszczol.one.pl/firmware/esp32.bin";
const char* config_url = "http://pszczol.one.pl/settings.json";
const char* report_url = "http://pszczol.one.pl/api/report_update.php";
const char* ssid = "AirPortExtreme";
const char* password = "Flash255";
const char* serverName = "http://pszczol.one.pl/api/add.php/";
String authHeader = "Basic bGFzZXI6bGFzZXIxMjM=";

HX711 scale;
uint8_t dataPin = 16;
uint8_t clockPin = 4;
float previous = 0;
bool firmwareUpdateFlag = false;
int loopDelay = 10;

void reportUpdate(bool success);

bool checkFirmwareUpdate() {
  WiFiClient client;
  HTTPClient http;
  Serial.println(" ");
  Serial.println("Sprawdzanie aktualizacji...");
  http.begin(client, firmware_url);

  int httpCode = http.GET();
  bool success = false;

  if (httpCode == 200) {
    int len = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

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
  http.end();
  reportUpdate(success);
  if (success) {
    ESP.restart();
  }
  return success;
}

void fetchConfig() {
  HTTPClient http;
  http.begin(config_url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      firmwareUpdateFlag = doc["firmwareUpdate"];
      loopDelay = doc["loopDelay"];
    }
  }
  http.end();
}

void reportUpdate(bool success) {
  HTTPClient http;
  http.begin(report_url);
  http.addHeader("Content-Type", "application/json");
  String payload = String("{\"success\":") + (success ? "true" : "false") + "}";
  http.POST(payload);
  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

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
  scale.set_offset(-598696);
  scale.set_scale(-25.353687);
  scale.tare();

  fetchConfig();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Utracono połączenie z WiFi. Ponowne łączenie...");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000);
    return;
  }

  fetchConfig();

  if (firmwareUpdateFlag) {
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

  String jsonPayload = "{\"weight\":" + String(weight) + "}";

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
