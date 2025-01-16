#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>

const char* ssid = "Pickova-CZNET.CZ"; // Název WiFi sítě
const char* password = "Pavlina72"; // Heslo WiFi sítě
const char* mqtt_server = "test.mosquitto.org"; // MQTT broker adresa

WiFiClient espClient;
PubSubClient client(espClient);

const int deepSleepDuration = 5 * 60 * 1000000; 
const int wakeUpPin = 33; 
void reconnect() {
  if (!client.connected()) {
    Serial.print("Zkouším MQTT připojení...");
    if (client.connect("ESP32Client")) {
      Serial.println("Připojeno k MQTT brokeru.");
    } else {
      Serial.print("Selhalo, rc=");
      Serial.print(client.state());
      Serial.println(". Zkouším znovu za 5 sekund.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Kontrola příčiny probuzení
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Probuzen signálem na GPIO.");
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Probuzen časovačem.");
  } else {
    Serial.println("Standardní probuzení.");
  }

  //WiFi
  WiFi.begin(ssid, password);
  Serial.print("Připojuji se k WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi připojeno!");
  Serial.print("IP adresa: ");
  Serial.println(WiFi.localIP());

  // MQTT
  client.setServer(mqtt_server, 1883);
  reconnect();

  //JSON
  StaticJsonDocument<200> doc;
  doc["device"] = "ESP32";
  doc["status"] = 1; 
  String jsonMessage;
  serializeJson(doc, jsonMessage);

  client.publish("outTopic", jsonMessage.c_str());
  Serial.println("Odeslána zpráva: " + jsonMessage);

  esp_sleep_enable_timer_wakeup(deepSleepDuration);       
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);           

  //Deep Sleep start
  Serial.println("ESP přechází do Deep Sleep.");
  esp_deep_sleep_start();
}

void loop() {
  
}
