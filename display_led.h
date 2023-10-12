#ifndef display_led_h
#define display_led_h

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

class DisplayLed
{
public:

    void init()
    {
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
    }

    void print(String text, int posx, int posy)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(posx,posy);
        display.println(text);
        display.display();
    }

    void start(String text)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("   Mulai");
        display.setCursor(40, 16);
        display.println("V");
        display.setCursor(55, 16);
        display.println(text);
        display.display();
    }

    void wifiCon(String text)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println(text);
        display.setCursor(0, 16);
        display.println("tersambung");
        display.display();
    }

    void connectingWiFi(String text)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.println("SSID :");
        display.setCursor(0,16);
        display.println(text);
        display.display();
    }

    void settingWiFi(int setWiFi)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.println("Set ulang WiFi ?");
        display.setCursor(80,16);
        display.println(setWiFi);
        display.display();
    }

    void weighCallib(int cnt_callib)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.println("Angkat    infus !");
        display.setCursor(90,16);
        display.println(cnt_callib);
        display.display();
    }

    void sample(int tpm, int weigh)
    {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(25,0);
        display.println(tpm);
        display.setCursor(70,0);
        display.println("TPM");
        display.setCursor(15,16);
        display.println(weigh);
        display.setCursor(60,16);
        display.println("gram");
        display.display();
    }

    // void sample(int tpm, int weigh)
    // {
    //     display.clearDisplay();
    //     display.setTextSize(2);
    //     display.setTextColor(SSD1306_WHITE);
    //     display.setCursor(0,0);
    //     display.println("TPM   :");
    //     display.setCursor(90,0);
    //     display.println(tpm);
    //     display.setCursor(0,16);
    //     display.println("Weigh :");
    //     display.setCursor(90,16);
    //     display.println(weigh);
    //     display.display();
    // }
};

#endif