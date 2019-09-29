#include <Arduino.h>
#include <SPIFFS.h>

#include "config.h"

#include <multiball/wifi.h>
#include <multiball/ota_updates.h>
#include <multiball/mqtt.h>

#include "leds.h"
#include "presets.h"
#include "animations.h"
#include "http_server.h"
#include "homebus_mqtt.h"

#ifdef HAS_BME280
#include <multiball/bme280.h>
#endif

#include "hw.h"

#ifdef BUILD_INFO

// CPP weirdness to turn a bare token into a string
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

char build_info[] = STRINGIZE(BUILD_INFO);
#else
char build_info[] = "not set";
#endif

// used to store persistent data across crashes/reboots
// cleared when power cycled or re-flashed
#ifdef ESP8266
int bootCount = 0;
#else
RTC_DATA_ATTR int bootCount = 0;
#endif

void setup() {
  const char* hostname = "";
  const char *wifi_credentials[] = {
    WIFI_SSID1, WIFI_PASSWORD1,
    WIFI_SSID2, WIFI_PASSWORD2,
    WIFI_SSID3, WIFI_PASSWORD3
  };

  bootCount++;

  delay(500);

  Serial.begin(115200);
  Serial.println("Hello World!");
  Serial.printf("Build %s\n", build_info);

  if(!SPIFFS.begin(true))
    Serial.println("An Error has occurred while mounting SPIFFS");
  else
    Serial.println("[spiffs]");

  if(wifi_begin(wifi_credentials, 3, "discoball")) {
    Serial.println(WiFi.localIP());
    Serial.println("[wifi]");

    hostname = wifi_hostname();

    if(!MDNS.begin(hostname))
      Serial.println("Error setting up MDNS responder!");
    else
      Serial.println("[mDNS]");

  } else {
    Serial.println("wifi failure");
  }

  ota_updates_setup();
  Serial.println("[ota_updates]");

  http_server_setup();
  Serial.println("[http_server]");

#ifdef USE_MQTT
  mqtt_setup(MQTT_HOST, MQTT_PORT, MQTT_UUID, MQTT_USER, MQTT_PASS);
  Serial.println("[mqtt]");

  homebus_mqtt_setup();
  Serial.println("[homebus-mqtt]");
#endif

  leds_setup();
  Serial.println("[leds]");

  preset_setup();
  Serial.println("[presets]");

  animation_setup();
  Serial.println("[animations]");

#ifdef HAS_BME280
  bme280_setup();
  Serial.println("[bme280]");
#endif
}

bool status_changed = true;

void loop() {
  wifi_handle();

  ota_updates_handle();

  http_server_handle();

#ifdef USE_MQTT
  mqtt_handle();
  homebus_mqtt_handle();
#endif

  leds_handle();

#ifdef HAS_BME280
  bme280_handle();
#endif
}
