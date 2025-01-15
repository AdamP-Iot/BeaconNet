#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>

const char* ssid = "SSID"; 
const char* password = "HESLO"; 
const char* mqtt_server = "test.mosquitto.org"; 

WiFiClient espClient;
PubSubClient client(espClient);

const int wakeUpPin = 33; 
const int deepSleepDuration = 60 * 1000000; 

void reconnect() {
  while (!client.connected()) {
    Serial.print("Zkouším MQTT připojení");
    if (client.connect("ESP32Client")) {
      Serial.println("připojeno");
    } else {
      Serial.print("selháno, rc=");
      Serial.print(client.state());
      Serial.println(" za 5 vteřin zkouším znovu");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); 

  bool isAwakeFromSignal = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0);
  if (isAwakeFromSignal) {
    Serial.println("Probuzen");
  } else {
    Serial.println("Basic probuzení.");
  }

  WiFi.begin(ssid, password);
  Serial.println("Připojuju se k wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("WiFi připojeno!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  reconnect();

  StaticJsonDocument<200> doc;
  doc["device"] = "ESP32";
  doc["status"] = isAwakeFromSignal ? 1 : 0; 
  String jsonMessage;
  serializeJson(doc, jsonMessage);

  client.publish("outTopic", jsonMessage.c_str());
  Serial.println("Zpráva: " + jsonMessage);

  Serial.println("ESP jde do deepsleep");
  delay(1000); 
  esp_deep_sleep_start();
}

void loop() {

}
