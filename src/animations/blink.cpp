#include "config.h"
#include "leds.h"
#include "animations.h"

unsigned animation_blink(bool init) {
  static CRGB old_leds[NUM_LEDS];
  static bool onoff = false;

  if(onoff) {
    for(int i = 0; i < NUM_LEDS; i++)
      leds[i] = old_leds[i];

    onoff = false;
  } else {
    for(int i = 0; i < NUM_LEDS; i++) {
      old_leds[i] = leds[i];
      leds[i] = 0;
    }

    onoff = true;
  }

  FastLED.show();

  return 1000;
}
