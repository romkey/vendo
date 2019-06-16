#include <WiFi.h>

#include "config.h"
#include "presets.h"
#include "animations.h"
#include "leds.h"
#include "mqtt.h"

static WiFiClient wifi_mqtt_client;
static PubSubClient mqtt_client(wifi_mqtt_client);

bool mqtt_connect() {
  if(mqtt_client.connected())
    return false;

  mqtt_client.connect(MQTT_UUID, MQTT_USER, MQTT_PASS);
  mqtt_client.subscribe(MQTT_CMD_TOPIC);
  return true;
}

void mqtt_setup() {
  mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_connect();
}

void mqtt_handle() {
  static unsigned long last_mqtt_check = 0;

  mqtt_client.loop();

  if(millis() > last_mqtt_check + 5000) {
    if(mqtt_connect())
      Serial.println("mqtt reconnect");

    last_mqtt_check = millis();
  }
}

void mqtt_callback(const char* topic, const byte* payload, unsigned int length) {
  char command[length + 1];

  memcpy(command, payload, length);
  command[length] = '\0';


  char buffer[length + 30];
  snprintf(buffer, length+30, "{ \"cmd\": \"%s\" }", command);

  mqtt_client.publish("/status", buffer);

  if(strcmp(command, "restart") == 0) {
    ESP.restart();
  }

  if(strcmp(command, "off") == 0) {
    leds_off();
    return;
  }

  if(strcmp(command, "on") == 0) {
    leds_on();
    return;
  }

  if(strcmp(command, "stop") == 0) {
    animation_stop();
    return;
  }

  if(strcmp(command, "start") == 0) {
    animation_start();
    return;
  }
}
