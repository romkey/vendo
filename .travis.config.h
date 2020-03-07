#pragma once

#define WIFI_SSID1     "wifi name 1"
#define WIFI_PASSWORD1 "wifi password 1"

#define WIFI_SSID2     "wifi name 2"
#define WIFI_PASSWORD2 "wifi password 2"

#define WIFI_SSID3     "wifi name 3"
#define WIFI_PASSWORD3 "wifi password 3"
#define NUM_LEDS 2

#ifdef ESP8266
#define LED_DATA_PIN D3
#define LED_RGB GRB
#else
#define LED_DATA_PIN 33
#define LED_RGB RGB
#endif

#define NUM_LEDS 100
#define LED_DATA_PIN 33
#define LED_TYPE WS2812
#define LED_RGB RGB
