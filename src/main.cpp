#include <Arduino.h>

#include <ESP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <ArduinoOTA.h>

// needed by BME280 library, not automatically included during PlatformIO build process :(
#include <Wire.h>
#include <SPI.h>

#include "config.h"
#include "leds.h"
#include "presets.h"
#include "animations.h"
#include "ota_updates.h"
#include "http_server.h"
#include "mqtt.h"

#ifdef HAS_BME280
#include "bme280.h"
#endif

#include "hw.h"

char hostname[sizeof("discoball-%02x%02x%02x") + 1];

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
static RTC_DATA_ATTR int bootCount = 0;
static RTC_DATA_ATTR int wifi_failures = 0;

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
  Serial.printf("Hostname: %s\n", hostname);
  Serial.printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
		mac_address[0],
		mac_address[1],
		mac_address[2],
		mac_address[3],
		mac_address[4],
		mac_address[5]);
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

  ota_updates_setup();
  Serial.println("[ota_updates]");

  http_server_setup();
  Serial.println("[http_server]");

  mqtt_setup();
  Serial.println("[mqtt]");

  leds_setup();
  Serial.println("[leds]");

#ifdef HAS_BME280
  bme280_setup();
#endif

#ifdef DISCOBALL_DISCO_1
  preset_set("pride");
  animation_set("march");
#endif

#ifdef DISCOBALL_CTRLH_1
  preset_set("white");
#endif
}


void loop() {
  ota_updates_handle();

  http_server_handle();

  mqtt_handle();
  
  leds_handle();

#ifdef HAS_BME280
  bme280_handle();
#endif
}
