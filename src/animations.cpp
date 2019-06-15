#include "leds.h"
#include "animations.h"

static unsigned march() {
  CRGB last = leds[0];

  for(int i = 0; i < NUM_LEDS - 1 ; i++)
    leds[i] = leds[i+1];

  leds[NUM_LEDS-1] = last;
  FastLED.show();

  return 1000;
}

static unsigned cylon() {

}

struct animation animations[] = {
  "march", march,
  "cylon", cylon
};

struct animation *current_animation  = NULL;
