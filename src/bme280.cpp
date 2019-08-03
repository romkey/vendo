#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// needed by BME280 library, not automatically included during PlatformIO build process :(
#include <Wire.h>
#include <SPI.h>

#include "wifi_local.h"
#include "mqtt.h"
#include "bme280.h"

#include "config.h"

#ifdef HAS_BME280

static Adafruit_BME280 bme280;
static bool is_present = false;

void bme280_setup() {
  if(!bme280.begin(0x76)) {
    Serial.println("BME280 not found");
  } else {
    is_present = true;
    Serial.println("[bme280]");
  }
}

void bme280_handle() {
}

float bme280_current_temperature() {
  return bme280.readTemperature();
}

float bme280_current_humidity() {
  return bme280.readHumidity();
}

float bme280_current_pressure() {
  return bme280.readPressure();
}
#endif
