#include "config.h"
#include "leds.h"
#include "animations.h"

unsigned animation_invert(bool init) {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = -leds[i];

  FastLED.show();

  return 1000;
}
