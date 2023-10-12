#ifndef koneksi_wifi_h
#define koneksi_wifi_h

#include "mem_set.h"
#include "koneksi_cred.h"

#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
// #include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "display_led.h"
#include "sensorinfus.h"
#include <esp_task_wdt.h>

// static const char DEFAULT_ROOT_CA[] =
// #include "certs/certloc_pem.h"

#define configWiFiButton 19

Button button_wifi;
DisplayLed displed_wifi;
DNSServer dnsServer;
AsyncWebServer server(80);
String avail_wifi, port_ssid, port_name, port_pass, port_token;
bool portal_on;
int setting_state;
WiFiClient client;

bool isButtonPressed = false;

String processor(const String &var)
{
  if (var == "wifi_selection")
  {
    return avail_wifi;
  }
  return String();
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

void buttonpressed()
{
  esp_task_wdt_init(1, true);
  esp_task_wdt_add(NULL);
  ESP.restart();
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

class ConnectionWiFi
{
private:
  String tokenid;
  String infusid;
  String send_message;
  HTTPClient http;

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

  int update_secure(InfusConfig &infusconfig, int tpm, int weigh)
  {
    Serial.println("Terhubung WiFi");

    // WiFiClientSecure *client = new WiFiClientSecure;
    int httpCode;

    tokenid = infusconfig.get(tokenID_p);
    infusid = infusconfig.get(infus_name_p);
    // client->setCACert(DEFAULT_ROOT_CA);
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;
      // BLYNK:
      // send_message = server_dom + send_p + token + tokenid + berat_v + String(weigh) + tpm_v + String(tpm);
      // Callmebot :
      // send_message = server_dom_callmebot + send_p_callmebot + get_p_callmebot + token_callmebot + tokenid + "&text=" + "ID+Device+=+" + String(infusid) + ";+TPM+=+" + String(tpm) + "+;+Weigh+=+" + String(weigh);
      // API :
      send_message = URL + prefixRoute + path + "?token=" + token_api + "&deviceId=" + String(infusid) + "&tpm=" + String(tpm) + "&weight=" + String(weigh);
      Serial.println(send_message);
      if (https.begin(client, send_message))
      // if (https.begin(*client, send_message))
      { // HTTPS
        // start connection and send HTTP header
        httpCode = https.GET();
        Serial.println(httpCode);

        // httpCode will be negative on error
        if (httpCode == 200)
        {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
          {
            String payload = https.getString();
            Serial.println(payload);
            displed_wifi.print("Kirim databerhasil", 0, 0);
          }
        }
        else
        {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          displed_wifi.print("Kirim datagagal", 0, 0);
        }

        https.end();
      }
    }
  return httpCode;
  }
};

#endif // !1
