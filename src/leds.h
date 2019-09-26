#pragma once

#include <FastLED.h>

#include "hw.h"

void leds_setup();
void leds_handle();
void leds_on();
void leds_off();
bool leds_status();
uint8_t leds_brightness(void);            // 0 - 100%
void leds_brightness(uint8_t brightness); // 0 - 100%

void leds_persist();
void leds_clear_persist();
void leds_restore();

void leds_fill(uint8_t red, uint8_t green, uint8_t blue);
void leds_fill(CRGB);

extern CRGB leds[NUM_LEDS];
