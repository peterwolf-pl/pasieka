#include <WiFi.h>
#include <HTTPClient.h>
#include "HX711.h"
#include <Update.h>

const char* firmware_url = "https://pszczol.one.pl/firmware/esp32_latest.bin";
const char* ssid = "AirPortExtreme";
const char* password = "Flash255";
const char* serverName = "http://pszczol.one.pl/api/add.php/";
String authHeader = "Basic bGFzZXI6bGFzZXIxMjM=";

HX711 scale;
uint8_t dataPin = 16;
uint8_t clockPin = 4;
float previous = 0;
/*
void checkFirmwareUpdate() {
  WiFiClient client;
  HTTPClient https;
 Serial.println(" ");
  Serial.println("Sprawdzanie aktualizacji...");
  https.begin(client, firmware_url);

https.begin(client, serverName);
https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);


  int httpCode = https.GET();

  if (httpCode == 200) {
    int len = https.getSize();
    WiFiClient* stream = https.getStreamPtr();

    if (Update.begin(len)) {
      size_t written = Update.writeStream(*stream);

      if (written == len && Update.end()) {
         Serial.println(" ");
        Serial.println("Aktualizacja zakończona sukcesem, restartuję...");
        ESP.restart();
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

  
}
*/
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

 // checkFirmwareUpdate();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Utracono połączenie z WiFi. Ponowne łączenie...");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000);
    return;
  }

  float weight = getStableWeight();
  int32_t rssi = WiFi.RSSI();

  Serial.print("Waga: ");
  Serial.print(weight);
  Serial.print(" g, RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  WiFiClient client;
  HTTPClient https;
  https.begin(client, serverName);
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Authorization", authHeader);

  String jsonPayload = "{\"weight\":" + String(weight) + "}";

 int httpResponseCode = https.POST(jsonPayload);
//
 Serial.print("HTTP Response code: ");
 Serial.println(httpResponseCode);
 Serial.println(" ");
//  Serial.println(" ");
//  https.end();
//  delay(60000);
https.end();
 delay(10000);
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
