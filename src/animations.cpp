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
  return 100;
}


struct animation animations[] = {
  "march", march,
  "cylon", cylon
};

struct animation *current_animation  = NULL;

void animation_start() {
}


void animation_stop() {
}

void animation_speed(int) {
}

animation_t *animation_lookup(const char* name) {
  for(int i = 0; i < sizeof(animations)/sizeof(animation); i++)
    if(strcmp(animations[i].name, name) == 0)
      return &animations[i];

  return NULL;
}

void animation_set(animation_t* animation) {
  current_animation = animation;
}

bool animation_set(const char* name) {
  animation_t *animation = animation_lookup(name);

  if(animation) {
    animation_set(animation);
    return true;
  }

  return false;
}
