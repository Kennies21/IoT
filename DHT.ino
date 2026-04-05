// -------  Nama Program : Monitoring suhu dan kelembapan -------
//------- Author : Keandra Indra Putra ------
//------- Versi : 1  ------
//------- Ownership : Pribadi------
//------- Deskripsi : suhu diatas 25 = led merah nyala dan dibawah 25 led biru nyala ------
//------- Depedency : - ------
#include <WiFi.h>
#include "DHTesp.h"

// ===== WIFI =====
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ===== THINGSPEAK =====
String apiKey = "9EGXIN6X5K8EXMEL";
const char* server = "api.thingspeak.com";

// ===== DHT22 =====
const int DHT_PIN = 19;
DHTesp dhtSensor;

// ===== LED =====
const int merah = 27;
const int biru = 25;

// ===== DATA =====
float suhu = 0;
float kelembaban = 0;

// ===== CLIENT =====
WiFiClient client;

void setup() {
  Serial.begin(115200);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  pinMode(biru, OUTPUT);
  pinMode(merah, OUTPUT);

  // ===== CONNECT WIFI =====
  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Terhubung!");
}

void loop() {

  // ===== BACA SENSOR =====
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  suhu = data.temperature;
  kelembaban = data.humidity;

  Serial.print("Temp: ");
  Serial.print(suhu);
  Serial.println(" °C");

  Serial.print("Hum: ");
  Serial.print(kelembaban);
  Serial.println(" %");

  // ===== LOGIKA LED =====
  if (suhu < 25) {
    digitalWrite(biru, HIGH);
    digitalWrite(merah, LOW);
  } else {
    digitalWrite(biru, LOW);
    digitalWrite(merah, HIGH);
    delay(500);
    digitalWrite(merah, LOW);
    delay(500);
  }

  // ===== KIRIM KE THINGSPEAK =====
  if (client.connect(server, 80)) {

    String url = "/update?api_key=" + apiKey +
                 "&field1=" + String(suhu) +
                 "&field2=" + String(kelembaban);

    Serial.println("\nMengirim ke ThingSpeak...");

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    // ===== TUNGGU RESPONSE =====
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Timeout!");
        client.stop();
        return;
      }
    }

    // ===== BACA RESPONSE =====
    Serial.println("Response:");
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }

    Serial.println("\nData terkirim!\n");

  } else {
    Serial.println("Gagal koneksi ke server");
  }

  client.stop();

  // ===== DELAY THINGSPEAK =====
  delay(15000);
}
