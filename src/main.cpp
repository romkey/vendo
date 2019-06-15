#include <Arduino.h>

#include <ESP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <ArduinoOTA.h>

#include <PubSubClient.h>

#include "config.h"
#include "leds.h"
#include "presets.h"
#include "animations.h"
#include "mqtt.h"
#include "hw.h"

static WiFiClient wifi_mqtt_client;
static PubSubClient mqtt_client(wifi_mqtt_client);

static char hostname[sizeof("discoball-%02x%02x%02x") + 1];
#ifdef BUILD_INFO

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

static char build_info[] = STRINGIZE(BUILD_INFO);
#else
static char build_info[] = "not set";
#endif

// used to store persistent data across crashes/reboots
// cleared when power cycled or re-flashed
static RTC_DATA_ATTR int bootCount = 0;
static RTC_DATA_ATTR int wifi_failures = 0;

CRGB leds[NUM_LEDS];

WiFiMulti wifiMulti;

void setup() {
  byte mac_address[6];

  bootCount++;

  delay(500);

  Serial.begin(115200);
  Serial.println("Hello World!");
  Serial.printf("Build %s\n", build_info);

  WiFi.macAddress(mac_address);
  snprintf(hostname, sizeof(hostname), "discoball-%02x%02x%02x", (int)mac_address[3], (int)mac_address[4], (int)mac_address[5]);
  Serial.printf("Hostname is %s\n", hostname);

  WiFi.setHostname(hostname);

  wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
  wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);
  wifiMulti.addAP(WIFI_SSID3, WIFI_PASSWORD3);

  static int wifi_tries = 0;
  while(wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);

    if(wifi_tries++ > 100) {
      wifi_failures++;
      ESP.restart();
    }
  }

  Serial.println(WiFi.localIP());
  Serial.println("[wifi]");

  if(!MDNS.begin(hostname))
    Serial.println("Error setting up MDNS responder!");
  else
    Serial.println("[mDNS]");

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      ESP.restart();
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
  mqtt_client.connect(MQTT_UUID, MQTT_USER, MQTT_PASS);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_client.subscribe(MQTT_CMD_TOPIC);
  Serial.println("[mqtt]");

  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_CLOCK_PIN, LED_RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  Serial.println("[leds]");
}

static unsigned long led_update_time = 0;
static unsigned long last_mqtt_check = 0;

void loop() {
  mqtt_client.loop();

  if(millis() > last_mqtt_check + 5000) {
    if(!mqtt_client.connected()) {
      mqtt_client.connect(MQTT_UUID, MQTT_USER, MQTT_PASS);
      mqtt_client.subscribe(MQTT_CMD_TOPIC);
      Serial.println("mqtt reconnect");
    }

    last_mqtt_check = millis();
  }

  ArduinoOTA.handle();

  if(current_animation && millis() > led_update_time) {
    led_update_time = millis() + (*current_animation->animation)();
  }
}
