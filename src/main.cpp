#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>
const char* ssid = "RIT-IOT";
const char* heslo = "NqUciiZ2Q8YLCur";
const char* mqttServer = "172.26.5.100";
WiFiClient espClient;
PubSubClient client(espClient);
const int wakeUpPiny[] = {32, 33, 25, 26};
const int cislo[] = {1, 2, 3, 4};
const char* pinNazvy[] = {"zelená", "žlutá", "červená", "houkačka"};
RTC_DATA_ATTR int prechoziHighPin = -1;
RTC_DATA_ATTR int pocetProbuzeni = 0;
#define PIN_32 (1ULL << GPIO_NUM_32)
#define PIN_33 (1ULL << GPIO_NUM_33)
#define PIN_25 (1ULL << GPIO_NUM_25)
#define PIN_26 (1ULL << GPIO_NUM_26)
uint64_t ext1Mask = PIN_32 | PIN_33 | PIN_25 | PIN_26;
const long long deepSleepDoba = 60LL * 60LL * 1000000;
const int maxPocetProbuzeni = 3;
void reconnect()
 { 
    if (!client.connected()) 
    {
        Serial.print("Zkouším MQTT připojení");
        if (client.connect("ESP32Client"))
        {
            Serial.println("Připojeno k MQTT brokeru.");
        } 
        else
        {
            Serial.print("Selhalo");
            Serial.print(client.state());
            Serial.println("Zkouším znovu za 5 sekund");
            delay(5000);
        }
    }
}
void sendStatusMessage(int pinIndex)
 {
    WiFi.begin(ssid, heslo);
    Serial.print("Připojuji se k WiFi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) 
    {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi připojeno!");
        client.setServer(mqttServer, 1883);
        reconnect();
        StaticJsonDocument<200> doc;
        doc["status"] = cislo[pinIndex];
        doc["pin_nazev"] = pinNazvy[pinIndex];
        String jsonMessage;
        serializeJson(doc, jsonMessage);
        client.publish("Adam_esp32/1", jsonMessage.c_str());
        Serial.println("Odeslána zpráva: " + jsonMessage);
    }
     else 
     {
        Serial.println("\nNepovedlo se připojit k WiFi včas.");
     } 
}
void setup()
{
    Serial.begin(115200);
    for (int i = 0; i < 4; i++)
    {
        pinMode(wakeUpPiny[i], INPUT_PULLDOWN);
    }
    esp_sleep_wakeup_cause_t wakeup_duvod = esp_sleep_get_wakeup_cause();
    if (wakeup_duvod == ESP_SLEEP_WAKEUP_EXT1 || wakeup_duvod == ESP_SLEEP_WAKEUP_TIMER)
    {
        uint64_t wakeUpBitmask = esp_sleep_get_ext1_wakeup_status();
        int aktivniPin = -1;
        for (int i = 0; i < 4; i++)
        {
            if ((wakeUpBitmask & (1ULL << wakeUpPiny[i])) != 0 || digitalRead(wakeUpPiny[i]) == HIGH)
            {
                aktivniPin = wakeUpPiny[i];
                aktivniPin = cislo[i];
                Serial.println("Aktivní pin: " + String(aktivniPin));
                if (aktivniPin == prechoziHighPin) 
                {
                    pocetProbuzeni++;
                    Serial.println("Stejný pin detekován"+ String(pocetProbuzeni) +  "x za sebou");
                    if (pocetProbuzeni >= maxPocetProbuzeni) 
                    {
                        Serial.println("Stejný pin detekován 3x za sebou");
                        prechoziHighPin = -1;
                        pocetProbuzeni = 0;
                    } 
                    else 
                    {
                        sendStatusMessage(i);
                    }
                } 
                else
                {
                    prechoziHighPin = aktivniPin;
                    pocetProbuzeni = 1;
                    sendStatusMessage(i);
                }               
                break;
            }
        }
    }
    if (pocetProbuzeni < maxPocetProbuzeni) 
    {
        esp_sleep_enable_timer_wakeup(deepSleepDoba);
    } 
    else
    {
        Serial.println("Časovač deaktivován, čekám na změnu signálu.");
    }
    esp_sleep_enable_ext1_wakeup(ext1Mask, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("ESP přechází do Deep Sleep");
    esp_deep_sleep_start();
}
void loop() 
{

}
