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

// Cylon is taken from FastLED examples https://github.com/FastLED/FastLED/blob/master/examples/Cylon/Cylon.ino
static void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

static unsigned cylon() {
  static int i = 0;
  static int8_t direction = +1;
  static int8_t hue = 0;

  leds[i] = CHSV(hue++, 255, 255);
  FastLED.show();
  fadeall();

  i += direction;
  if(i == NUM_LEDS - 1)
    direction = -1;

  if(i == 0)
    direction = +1;

  return 10;
}

animation_t animations[] = {
  "march", march,
  "cylon", cylon
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
}

bool animation_set(const char* name) {
  animation_t* animation = animation_lookup(name);

  if(animation) {
    animation_set(animation);
    return true;
  }

  return false;
}
