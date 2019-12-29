#include <Arduino.h>
#include <SPIFFS.h>

#include "config.h"

#include "multiball.h"
#include <multiball/app.h>
#include <multiball/homebus.h>

#include "leds.h"
#include "presets.h"
#include "animations.h"
#include "http_server.h"

#include "vendo.h"

#include <multiball/bme280.h>

#include "hw.h"

MultiballApp App;

void setup() {
  const wifi_credential_t wifi_credentials[] = {
    { WIFI_SSID1, WIFI_PASSWORD1 },
    { WIFI_SSID2, WIFI_PASSWORD2 },
    { WIFI_SSID3, WIFI_PASSWORD3 }
  };

  delay(500);

  App.wifi_credentials(3, wifi_credentials);
  App.begin("discoball");

  http_server_setup();
  Serial.println("[http_server]");

  homebus_configure("Discoball", "", "Homebus", "1");
  homebus_setup();

  vendo_setup();
  Serial.println("[homebus-mqtt]");

  leds_setup();
  Serial.println("[leds]");

  preset_setup();
  Serial.println("[presets]");

  animation_setup();
  Serial.println("[animations]");

  bme280_setup();
  Serial.println("[bme280]");
}

void loop() {
  App.handle();
  homebus_handle();

  vendo_handle();

  http_server_handle();

  leds_handle();

  bme280_handle();
}
