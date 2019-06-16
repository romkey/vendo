#include "config.h"
#include "mqtt.h"

bool mqtt_connect(PubSubClient* mqtt_client) {
  if(mqtt_client->connected())
    return false;

  mqtt_client->connect(MQTT_UUID, MQTT_USER, MQTT_PASS);
  mqtt_client->subscribe(MQTT_CMD_TOPIC);
  return true;
}

void mqtt_handle() {
  static unsigned long last_mqtt_check = 0;

  mqtt_client.loop();

  if(millis() > last_mqtt_check + 5000) {
    if(mqtt_connect(&mqtt_client))
      Serial.println("mqtt reconnect");

    last_mqtt_check = millis();
  }
}

void mqtt_callback(const char*, const byte*, unsigned) {
}

