#ifndef koneksi_wifi_h
#define koneksi_wifi_h

#include "mem_set.h"
#include "koneksi_cred.h"
#include "display_led.h"
#include "sensorinfus.h"

#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <esp_task_wdt.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

Button button_wifi;
DisplayLed displed_wifi;
DNSServer dnsServer;
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

String avail_wifi, port_ssid, port_pass;
bool portal_on;
int configWiFiButton = 19;

void buttonpressed()
{
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  ESP.restart();
}

String processor(const String &var)
{
  if (var == "wifi_selection")
  {
    return avail_wifi;
  }
  return String();
}

void checkavailnetwork()
{
  avail_wifi = "SSID: <select name = wifi_ssid>";

  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    avail_wifi += "<option value=null>No Network Available</option><br>";
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      String nama_wifi = "'"+WiFi.SSID(i)+"'";
      avail_wifi += "<option value=" + nama_wifi + ">" + WiFi.SSID(i) + "</option><br>";
    }
  }
  avail_wifi += "</select><br>";
}

void setupServer(class InfusConfig &config)
{
  checkavailnetwork();
  server.on(
      "/",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        request->send(LittleFS, "/edit_data.htm", String(), false, processor);
        Serial.println("Client Connected");
      });

  server.on(
      "/save_data",
      HTTP_GET,
      [](AsyncWebServerRequest *request)
      {
        if (request->hasParam("wifi_ssid"))
        {
          port_ssid = request->getParam("wifi_ssid")->value();
        }
        if (request->hasParam("wifi_pass"))
        {
          port_pass = request->getParam("wifi_pass")->value();
        }
        request->send(200, "text/plain", "Informasi tersimpan. Akhiri Sesi");
        portal_on = 0;
      }
    );
}

bool start_portal(InfusConfig &config)
{
  button_wifi.init(configWiFiButton);
  attachInterrupt(configWiFiButton, buttonpressed, FALLING);

  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP_STA);
  Serial.print(F("SSID AP : "));
  Serial.println(config.get(infus_name_p).c_str());

  WiFi.softAP(config.get(infus_name_p).c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  setupServer(config);
  Serial.println("selesai setup server");
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());

  // server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
  // more handlers...
  server.begin();
  portal_on = 1;
  
  Serial.println(portal_on);

  while (portal_on)
  {
    dnsServer.processNextRequest();
    delay(100);
  }
  server.end();
  // Close Server

  config.edit(wifi_pass_p, port_pass);
  config.edit(wifi_ssid_p, port_ssid);
  config.save(LittleFS);
  WiFi.mode(WIFI_OFF);
  detachInterrupt(configWiFiButton);
  return 0;
}

class CaptiveRequestHandler : public AsyncWebHandler
{
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
      request->addInterestingHeader("Setting Infus");
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
      request->send(LittleFS, "/edit_data.htm", String(), false, processor);
    }
};

class ConnectionWiFi
{
  private:
    String tokenid;
    String infusid;
    String send_message;
    const char* mqttServer = "192.168.1.23"; // Alamat broker MQTT
    const int mqttPort = 1883; // Port MQTT default
    const char* mqttUsername = ""; // Username MQTT (jika ada)
    const char* mqttPassword = ""; // Password MQTT (jika ada)
    const char* topic = "rivaldm/test";

  public:
    bool checkwifi()
    {
      int limit_try = 20, cnt = 0;
      while (WiFi.status() != WL_CONNECTED && cnt < limit_try)
      {
        Serial.println("Connecting WiFi...");
        cnt ++;
        delay(250);
      }
      // Check koneksi
      if(cnt < limit_try){
        return 1;
      }
      // Return 0 jika tidak bisa koneksi
      return 0;
    }
  
    void connectWifi(InfusConfig &infusconfig)
    {
      displed_wifi.connectingWiFi(infusconfig.get(wifi_ssid_p).c_str());
      WiFi.mode(WIFI_STA);
      WiFi.begin(infusconfig.get(wifi_ssid_p).c_str(), infusconfig.get(wifi_pass_p).c_str());
      delay(500);
    }

    void setupMqtt()
    {
      client.setServer(mqttServer, mqttPort);
    }

    int reconnect()
    {
      Serial.print("Attempting MQTT connection...");
      // ID klien harus unik. Biasanya, Anda dapat menggunakan MAC address ESP32.
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      if (client.connect(clientId.c_str(), mqttUsername, mqttPassword))
      {
        Serial.println("connected");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
      return client.state();
    }

    void sendWiFi(InfusConfig &infusconfig, int tpm, int weigh)
    {
      if (!client.connected())
      {
        reconnect();
      }
      client.loop();

      // Kirim pesan MQTT
      infusid = infusconfig.get(infus_name_p);
      StaticJsonDocument<200> doc;
      doc["ID_Infus"] = infusid;
      doc["TPM"] = tpm;
      doc["Weight"] = weigh;

      String message;
      serializeJsonPretty(doc, message);

      Serial.println(message);
      client.publish(topic, message.c_str());
      delay(3000);
    }
};

#endif // !1