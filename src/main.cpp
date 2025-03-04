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
const long long deepSleepDoba = 10LL * 1000000;
const int wakeUpPiny[]  = {32, 33, 25, 26};
const char* pinNazvy[] = {"zelená", "žlutá", "červená", "houkačka"};
RTC_DATA_ATTR int prechoziHighPin = -1;
RTC_DATA_ATTR int pocetProbuzeni  = 0;
#define PIN_32 (1ULL << GPIO_NUM_32)
#define PIN_33 (1ULL << GPIO_NUM_33)
#define PIN_25 (1ULL << GPIO_NUM_25)
#define PIN_26 (1ULL << GPIO_NUM_26)
uint64_t ext1Mask = PIN_32 | PIN_33 | PIN_25 | PIN_26; 
void reconnect()
 {
  if (!client.connected())
   {
    Serial.print("Zkouším MQTT připojení...");
    if (client.connect("ESP32Client"))
     {
      Serial.println("Připojeno k MQTT brokeru.");
    }
     else
      {
      Serial.print("Selhalo, rc=");
      Serial.print(client.state());
      Serial.println(". Zkouším znovu za 5 sekund.");
      delay(5000);
    }
  }
}
void sendStatusMessage(int pinIndex) 
{
  WiFi.begin(ssid, heslo);
  Serial.print("Připojuji se k WiFi");
  unsigned long trvavniPripojeni = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - trvavniPripojeni < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println("\nWiFi připojeno!");
    Serial.print("IP adresa: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqttServer, 1883);
    reconnect();
    StaticJsonDocument<200> doc;
    doc["status"] = 1;
    doc["pin_nazev"] = pinNazvy[pinIndex];
    String jsonMessage;
    serializeJson(doc, jsonMessage);
    client.publish("outTopic", jsonMessage.c_str());
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
    pinMode(wakeUpPiny[i], INPUT); 
  }
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) 
  {
    Serial.println("Probuzen EXT1.");
    delay(50);
    bool signalDetected = false;
    for (int i = 0; i < 4; i++) 
    {
      if (digitalRead(wakeUpPiny[i]) == HIGH) 
      {
        Serial.println("Probuzen signálem na GPIO " + String(wakeUpPiny[i]));
        if (wakeUpPiny[i] == prechoziHighPin)
         {
          pocetProbuzeni++;
          Serial.println("Stejný signál detekován " + String(pocetProbuzeni));
          if (pocetProbuzeni >= 3) 
          {
            Serial.println("Signál nezměněn 3x. Čekám na nový pin.");
            prechoziHighPin = -1;
            pocetProbuzeni  = 0;
          }
           else 
          {
            sendStatusMessage(i);
          }
        } 
        else 
        {
          Serial.println("Nový signál. Reset počítadla.");
          prechoziHighPin = wakeUpPiny[i];
          pocetProbuzeni  = 0;
          sendStatusMessage(i);
        }
        signalDetected = true;
        break;
      }
    }
    if (!signalDetected)
    {
      Serial.println("EXT1 probuzení, ale žádný pin není HIGH.");
      prechoziHighPin = -1;
      pocetProbuzeni  = 0;
    }
  }
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) 
  {
    Serial.println("Probuzen časovačem.");
    bool signalDetected = false;
    for (int i = 0; i < 4; i++) {
      if (digitalRead(wakeUpPiny[i]) == HIGH) 
      {     
        if (wakeUpPiny[i] == prechoziHighPin)
         {
          pocetProbuzeni++;
          Serial.println("Stejný signál (" + String(pocetProbuzeni) + "x).");
          if (pocetProbuzeni >= 3)
           {
            Serial.println("Signál beze změny 3x. Čekám na nový signál.");
            prechoziHighPin = -1;
            pocetProbuzeni  = 0;
          } 
          else 
          {
            sendStatusMessage(i);
          }
        } 
        else 
        {
          Serial.println("Nový signál. Reset počítadla.");
          prechoziHighPin = wakeUpPiny[i];
          pocetProbuzeni  = 0;
          sendStatusMessage(i);
        }
        signalDetected = true;
        break;
      }
    }
    if (!signalDetected) 
    {
      Serial.println("Probuzen časovačem");
      prechoziHighPin = -1;
      pocetProbuzeni  = 0;
    }
  }
  if (pocetProbuzeni < 3) 
  {
    esp_sleep_enable_timer_wakeup(deepSleepDoba);
  } 
  else
   {
    Serial.println("Časovač vypnut. Čekám na nový signál.");
  }
  esp_sleep_enable_ext1_wakeup(ext1Mask, ESP_EXT1_WAKEUP_ANY_HIGH);
  Serial.println("ESP přechází do Deep Sleep...");
  esp_deep_sleep_start();
}
void loop() 
{

}
