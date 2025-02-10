#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>
const char* ssid = "RIT-IOT";
const char* hesloWifi = "NqUciiZ2Q8YLCur";
const char* mqttServer = "172.26.5.100";
WiFiClient espClient;
PubSubClient client(espClient);
const uint64_t deepSleepTrvani = 30ULL * 1000000; 
const int wakeUpPiny[] = {32, 33, 25, 26};
const char* pinNazvy[] = {"zelená", "žlutá", "červená", "houkačka"};
RTC_DATA_ATTR int posledniHighPin = -1; 
RTC_DATA_ATTR int stejnySignalPocet = 0; 
void pripojeni() {
  int znovu = 0;   
  while (!client.connected() && znovu < 5) {  // Zkusíme 5krát připojit
      Serial.print("Zkouším MQTT připojení...");
      if (client.connect("ESP32Client")) {
          Serial.println("Připojeno k MQTT brokeru.");
          return;
      } else {
          Serial.print("Selhalo, rc=");
          Serial.println(client.state());
          delay(3000);
          znovu++;
      }
  }
  Serial.println("Nepodařilo se připojit k MQTT!");
}

void posliStatus(int pinIndex) 
{
  WiFi.begin(ssid, hesloWifi);
  Serial.print("Připojuji se k WiFi");
  while (WiFi.status() != WL_CONNECTED)
   {
    delay(3000);
    Serial.print(".");
  }
  Serial.println("\nWiFi připojeno!");
  Serial.print("IP adresa: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttServer, 1883);
  pripojeni();
  StaticJsonDocument<200> doc;
  doc["pin_nazev"] = pinNazvy[pinIndex]; 
  doc["pin_cislo"] = pinIndex + 1;
  String jsonMessage;
  serializeJson(doc, jsonMessage);
  client.publish(" /1", jsonMessage.c_str());
  Serial.println("Odeslána zpráva: " + jsonMessage);
  delay(500);
}
void setup() 
{
  Serial.begin(115200);
  for (int i = 0; i < 4; i++)
   {
    pinMode(wakeUpPiny[i], INPUT);
  }
  esp_sleep_wakeup_cause_t wakeupDuvod = esp_sleep_get_wakeup_cause();
  if (wakeupDuvod == ESP_SLEEP_WAKEUP_EXT1)
   {
    Serial.println("Probuzen signálem na GPIO.");
    for (int i = 0; i < 4; i++) {
      if (digitalRead(wakeUpPiny[i]) == HIGH)
       {
         delay(50);
         if (posledniHighPin == wakeUpPiny[i])
         {
           stejnySignalPocet++;
           Serial.println("Stejný signál detekován. Počet: " + String(stejnySignalPocet));         
        } else
         {
          Serial.println("Nový signál detekován na GPIO" + String(wakeUpPiny[i]));
          posledniHighPin = wakeUpPiny[i];
          stejnySignalPocet = 1; 
          posliStatus(i);
        }
        break;
      }
    }
  } else if (wakeupDuvod == ESP_SLEEP_WAKEUP_TIMER) 
  {
    Serial.println("Probuzen časovačem.");
    bool PinyHigh = false;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(wakeUpPiny[i]) == HIGH)
       {
        if (posledniHighPin == wakeUpPiny[i]) 
        {         
          stejnySignalPocet++;
          Serial.println("Časovač detekoval stále stejný signál na GPIO" + String(wakeUpPiny[i]));
          posliStatus(i);
          PinyHigh = true;
          break;          
        } else
         {
          Serial.println("Časovač detekoval nový signál na GPIO" + String(wakeUpPiny[i]));
          posledniHighPin = wakeUpPiny[i];
          stejnySignalPocet = 1;
          posliStatus(i);
          PinyHigh = true;
          break;
        }
      }
    }
    if (!PinyHigh)
     {
      Serial.println("Časovač nenašel žádný HIGH signál.");
      posledniHighPin = -1; 
      stejnySignalPocet = 0; 
    }
  } else
   {
    Serial.println("Standardní probuzení.");
  }
  esp_sleep_enable_timer_wakeup(deepSleepTrvani); 
  // Povolení probouzení na více GPIO pinech
  uint64_t maska = 0;
  for (int i = 0; i < 4; i++)
   {
    if (wakeUpPiny[i] != posledniHighPin || stejnySignalPocet == 0)
     { 
      maska |= (1ULL << wakeUpPiny[i]); // Povolit všechny kromě posledního pinu
    }
  }
  esp_sleep_enable_ext1_wakeup(maska, ESP_EXT1_WAKEUP_ANY_HIGH);
  Serial.println("ESP přechází do Deep Sleep.");
  esp_deep_sleep_start();
}
void loop() 
{
  
}
