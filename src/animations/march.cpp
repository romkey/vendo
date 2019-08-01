#include "config.h"
#include "leds.h"
#include "animations.h"

unsigned animation_march(bool init) {
  CRGB last = leds[0];

  if(init)
    return 0;

  for(int i = 0; i < NUM_LEDS - 1 ; i++)
    leds[i] = leds[i+1];

  leds[NUM_LEDS-1] = last;
  FastLED.show();

  return 1000;
}
