//-----------ESP32
#include "mem_set.h"
#include "sensorinfus.h"
#include "koneksi_wifi.h"
#include "koneksi_sim.h"
#include "display_led.h"
#include "buzzer.h"
#include "otadrive_esp.h"
#include <soc/rtc_wdt.h>

//-----------Buat object dari class
InfusConfig config1;
ConnectionWiFi connect1;
ConnectionSIM sim;
Tpm tpm;
Weigh weigh;
LoadCellConfig loadconfig;
Button button;
DisplayLed displed;
buzzer buzz;
Bat bat;

//-----------Inisialisasi pin dan variabel
#define tpm_pin 18
#define LOADCELL_DOUT_PIN 4
#define LOADCELL_SCK_PIN 2
#define configWiFiButton 19
#define pinBat 35
#define pinBuzz 5
bool pauseState;
bool pauseBeep;
unsigned long buttonPressStartTime = 0;
String firmwareVersion = "0.0";

void updatetpm()
{
  tpm.update();
}

void beginsens(){
    tpm.init(tpm_pin);
    attachInterrupt(tpm_pin, updatetpm, FALLING);
    weigh.init(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void pauseMonitoring(){
    if(button.is_push()){
        unsigned long currentTime = millis();
        if ((currentTime - buttonPressStartTime) >= 500) {
            Serial.println("PAUSE");
            detachInterrupt(configWiFiButton);
            pauseState = HIGH;
            pauseBeep = HIGH;
        }
    }
}

void setup(){
    //-----------STEP0: begin serial communication
    Serial.begin(115200);
    
    //-----------STEP1: Init
    button.init(configWiFiButton);
    displed.init();
    init_fs();
    buzz.init(pinBuzz);
    bat.init(pinBat);
    OTADRIVE.setInfo("ab188fb1-8379-4d6d-a2c2-d093593cc91e", firmwareVersion);
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    
    //-----------STEP2: Load Config
    displed.start(firmwareVersion);
    delay(1000);
    displed.print("Cek memoriWiFi...", 0, 0);
    config1.load(LittleFS);
    config1.print();

    //-----------STEP3: Init Connection
    connect1.connectWifi(config1);

    //-----------STEP4: Config if needed and check wifi connection
    if(connect1.checkwifi()){
        Serial.println("WiFi Connected");
        String wifi = config1.get(wifi_ssid_p);
        Serial.println(wifi);
        wifi = wifi.substring(0,10);
        displed.wifiCon(wifi);
        buzz.buzzbeep(500);
        delay(1500);
    }else{
        Serial.println("Wifi Not Connected");
        displed.print("WiFi tidaktersambung", 0, 0);
        delay(2000);
        Serial.println("Starting Captive Portal...");
        displed.print("Mengatur  WiFi...", 0, 0);
        buzz.buzzbeep(500);
        start_portal(config1);
        delay(500);
        connect1.connectWifi(config1);
        if (connect1.checkwifi()){
            Serial.println("WiFi Connected");
            String wifi = config1.get(wifi_ssid_p);
            Serial.println(wifi);
            wifi = wifi.substring(0,10);
            displed.wifiCon(wifi);
            buzz.buzzbeep(500);
            delay(1500);
        } else {
            Serial.println("Wifi Not Connected");
            displed.print("WiFi tidaktersambung", 0, 0);
            delay(2000);
            displed.print("Restart...", 0, 0);
            buzz.buzzbeep(500);
            delay(1500);
            esp_task_wdt_init(1, true);
            esp_task_wdt_add(NULL);
            ESP.restart();
        }
        vTaskDelay(1);
    }
    // Hitung mundur dari 5 untuk opsi penggantian WiFi
    int cnt_config = 5;
    unsigned long previousMillis = 0;
    unsigned long interval = 1000;
    while(cnt_config > -1 && !button.is_push()){
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
    
            if (cnt_config > -1) {
                Serial.print("Setting WiFi ? ");
                Serial.println(cnt_config);
                displed.settingWiFi(cnt_config);
                cnt_config--;
            }       
        }
    }
    if (cnt_config > -1){
        Serial.println("Starting Captive Portal...");
        displed.print("Mengatur  WiFi...", 0, 0);
        buzz.buzzbeep(500);
        start_portal(config1);
        delay(500);
        connect1.connectWifi(config1);
        if (!connect1.checkwifi()){
            Serial.println("Wifi Not Connected");
            displed.print("WiFi tidaktersambung", 0, 0);
            delay(2000);
            displed.print("Restart...", 0, 0);
            buzz.buzzbeep(500);
            delay(1500);
            esp_task_wdt_init(1, true);
            esp_task_wdt_add(NULL);
            ESP.restart();
        }
        vTaskDelay(1);
    }
    // Info Wifi tersambung kemana
    Serial.println("WiFi Connected");
    String wifi = config1.get(wifi_ssid_p);
    Serial.println(wifi);
    wifi = wifi.substring(0,10);
    displed.wifiCon(wifi);
    buzz.buzzbeep(500);
    delay(500);
    config1.print();
    // config1.edit(tokenID_p, "2nrtIgwDCHP5SF3CToAWWdWZFPGtz6oX");
    // config1.save(LittleFS);

    // Cek update firmware
    displed.print("Cek updatefirmware", 0, 0);
    OTADRIVE.updateFirmware();

    //-----------STEP5: Setup SIM Card
    Serial.println("Setup SIM...");
    displed.print("MenyiapkanSIM...", 0, 0);
    sim.init();
    buzz.buzzbeep(500);
    delay(1500);

    //-----------STEP6: Init Sensor
    beginsens();
    
    //--------------Callib and Save----------------------
    // weigh.callib(); // Lakukan proses callib, atau Load
    // Serial.print(weigh.get_scale());
    // loadconfig.edit(weigh.get_scale());

    // //Save to Config
    // loadconfig.save(LittleFS);
    //---------------------------------------------------

    //-------------Load and Callibr---------------------
    // int weigh_callib_lim = 0, weigh_callib = 10;
    // while(weigh_callib > weigh_callib_lim){
    //     Serial.println(weigh_callib);
    //     displed.weighCallib(weigh_callib);
    //     buzz.buzzbeep(500);
    //     delay(500);
    //     weigh_callib--;
    // }
    displed.print("Jangan    gantung", 0, 0);
    buzz.buzzbeep(1000);
    displed.print("infus !", 0, 0);
    delay(1000);
    while(!button.is_push()){
        displed.print("Infus tak digantung?", 0, 0);
        if(button.is_push()){
            break;
        }
    }
    displed.print("Kalibrasi berat...", 0, 0);

    loadconfig.load(LittleFS);
    Serial.println("");
    weigh.set_callib(loadconfig.get());
    Serial.printf("Load Param: %f", loadconfig.get());
    Serial.println("");

    displed.print("Gantung   infus !", 0, 0);
    buzz.buzzbeep(500);
    delay(1000);
    //-------------------------------------------------

    //Konfirmasi mulai monitoring
    while(!button.is_push()){
        displed.print("Infus     berjalan ?", 0, 0);
        if(button.is_push()){
            break;
        }
    }
    displed.print("Monitoringdimulai",0 ,0);
    buzz.buzzbeep(500);
    delay(500);
}

void loop() {
    //State Monitoring

    //Interrupt for Pause
    attachInterrupt(configWiFiButton, pauseMonitoring, FALLING);
    
    //-----------STEP-M1: Get Sensor Data & Displaying
    int val_sample_berat = weigh.get_unit();
    // int val_sample_berat = random(0, 750);
    if (val_sample_berat < 0){
      val_sample_berat = 0;
    }
    int val_sample_tpm = tpm.get();
    // int val_sample_tpm = random(0,100);
    Serial.print("TPM: ");
    Serial.println(val_sample_tpm);
    Serial.print("Weigh: ");
    Serial.println(val_sample_berat);
    
    displed.sample(val_sample_tpm, val_sample_berat);

    delay(2000);

    // connect1.connectWifi(config1);

    //-----------STEP-M2: Connection Management & Send Data
    if(connect1.checkwifi()){
        if(connect1.update_secure(config1, val_sample_tpm, val_sample_berat) != 200){
            buzz.buzzbeep(500);
            delay(2000);
        } else {
            delay(2500);
        }
    }else{ //Cek bisa sim atau tidak
        displed.print("WiFi tidaktersambung", 0, 0);
        delay(2000);
        displed.print("Kirim datavia SIM", 0, 0);
        Serial.println("KONEKSI SIM");
        vTaskDelay(1);
        if (sim.connect_gprs(config1, val_sample_tpm, val_sample_berat) != 200){
            displed.print("Kirim datagagal", 0, 0);
            buzz.buzzbeep(500);
            delay(500);
        } else {
            delay(1000);
        }
    }
    //Cek kondisi baterai
    if (bat.cek()){
        displed.print("Battery   Low", 0, 0);
        buzz.buzzbeep(1000);
    }
    
    //Cek tombol pause ditekan atau tidak
    while(pauseState == HIGH){
        displed.print("  PAUSED", 0, 0);
        if(pauseBeep == HIGH){
            buzz.buzzbeep(500);
        }
        if(pauseBeep == HIGH){
            pauseBeep = LOW;
        }
        if(button.is_push()){
            delay(1000);
            detachInterrupt(configWiFiButton);
            pauseState = LOW;
        }
    }
}
