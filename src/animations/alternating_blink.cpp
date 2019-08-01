#include "config.h"
#include "leds.h"
#include "animations.h"

unsigned animation_alternating_blink(bool init) {
  static CRGB old_leds[NUM_LEDS];
  static uint8_t offset = 0;

  if(init) {
    for(int i = 0; i < NUM_LEDS; i++)
      old_leds[i] = leds[i];

    return 0;
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    if((i + offset) % 2)
      leds[i] = old_leds[i];
    else
      leds[i] = 0;
  }

  FastLED.show();

  offset +=1;
  offset %= 2;

  return 1000;
}
