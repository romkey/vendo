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

static unsigned throb() {
  static uint8_t brightness = 255;
  static int8_t direction = -1;
  static CRGB old_leds[NUM_LEDS];
  static bool first = true;

  if(first) {
    for(int i = 0; i < NUM_LEDS; i++)
      old_leds[i] = leds[i];

    first = false;
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = old_leds[i];
    leds[i].nscale8_video(brightness);
  }

  FastLED.show();

  if(brightness == 255)
    direction = -1;

  if(brightness == 0)
    direction = 1;

  brightness += direction;

  return 10;
}

static unsigned invert() {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = -leds[i];

  FastLED.show();

  return 1000;
}

static unsigned blink() {
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

static unsigned alternating_blink() {
  static CRGB old_leds[NUM_LEDS];
  static uint8_t offset = 0;
  static bool first = true;

  if(first)
    for(int i = 0; i < NUM_LEDS; i++)
      old_leds[i] = leds[i];

  for(int i = 0; i < NUM_LEDS; i++) {
    CRGB temp = leds[i];

    leds[i] = old_leds[i];
    old_leds[i] = temp;
  }

  offset +=1;
  offset %= 2;

  FastLED.show();

  return 1000;
}

animation_t animations[] = {
  "blink", blink,
  "alternating blink", alternating_blink,
  "invert", invert,
  "march", march,
  "throb", throb
};

unsigned animations_length = sizeof(animations)/sizeof(animation_t);

struct animation* current_animation  = NULL;

static float speed = 1;
static bool running = true;

void animation_start() {
  running = true;
}

void animation_stop() {
  running = false;
}

void animation_speed(float desired_speed) {
  speed = desired_speed;
}

unsigned animate() {
  if(running)
    return (unsigned)((*current_animation->animation)() / speed);
  else
    return 10;
}

animation_t* animation_lookup(const char* name) {
  for(int i = 0; i < sizeof(animations)/sizeof(animation); i++)
    if(strcmp(animations[i].name, name) == 0)
      return &animations[i];

  return NULL;
}

void animation_set(animation_t* animation) {
  current_animation = animation;
  animation_start();
}

bool animation_set(const char* name) {
  animation_t* animation = animation_lookup(name);

  if(animation) {
    animation_set(animation);
    return true;
  }

  return false;
}
