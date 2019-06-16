#include <FastLED.h>

#include "hw.h"

void leds_setup();
void leds_handle();
void leds_on();
void leds_off();
void leds_brightness(uint8_t brightness); // 0 - 100%

extern CRGB leds[NUM_LEDS];
