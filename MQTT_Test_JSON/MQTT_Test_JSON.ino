#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "ZTE_2.4G_5yzkX4";
const char* password = "123456789";
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "rivaldm/test";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi...");
  }

  Serial.println("Terhubung ke WiFi");

  // Inisialisasi MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  // Data JSON yang akan Anda kirim
//  String jsonMessage = 
//  "{\"sensor\":\"DHT22\",\"temperature\":25.5,\"humidity\":45.3}";
  int nomer = random(0, 100);
  StaticJsonDocument<200> doc;
  doc["nama"] = "John";
  doc["umur"] = nomer;

  String jsonMessage;
  serializeJsonPretty(doc, jsonMessage);

  if (client.publish(mqtt_topic, jsonMessage.c_str())) {
    Serial.println("Pesan terkirim ke MQTT broker");
  } else {
    Serial.println("Gagal mengirim pesan");
  }

  delay(5000); // Tunda pengiriman pesan
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Mencoba terhubung ke MQTT broker...");
    if (client.connect("ESP32Client")) {
      Serial.println("Terhubung ke MQTT broker");
    } else {
      Serial.print("Gagal terhubung, status: ");
      Serial.print(client.state());
      Serial.println(" Coba kembali dalam 5 detik");
      delay(5000);
    }
  }
}
