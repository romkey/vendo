#pragma once

// need to include any library files used by the multiball libraries
// limitation of platformio
#include <Wire.h>
#include <SPI.h>

#include <Ticker.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// no longer used - moved to individual configurations in config.h
#if 0
// LED strip configuration
#define NUM_LEDS 100

#define LED_DATA_PIN 33
// #define LED_CLOCK_PIN 32
// #define LED_TYPE LPD8806
#define LED_TYPE WS2812B
// #define LED_RGB BRG
#define LED_RGB RGB
#endif
