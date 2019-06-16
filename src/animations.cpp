#include "leds.h"
#include "animations.h"

void animation_start() {
}


void animation_stop() {
}

void animation_speed() {
}

static unsigned march() {
  CRGB last = leds[0];

  for(int i = 0; i < NUM_LEDS - 1 ; i++)
    leds[i] = leds[i+1];

  leds[NUM_LEDS-1] = last;
  FastLED.show();

  return 1000;
}

static unsigned cylon() {
  return 100;
}


struct animation animations[] = {
  "march", march,
  "cylon", cylon
};

struct animation *current_animation  = NULL;
