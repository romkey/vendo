#include "config.h"
#include "leds.h"
#include "animations.h"
#include "presets.h"

extern bool status_changed;


/*
 * PRESETS
 *
 * Presets sinply set the LEDs to a selected color or pattern.
 *
 * The preset routine should call FastLED.show() when it's done.
 *
 * Setting a preset stops the current animation.
 */

static void white() {
  leds_fill(CRGB::White);
  
  FastLED.show();
}

static void pink() {
  leds_fill(CRGB::Pink);

  FastLED.show();
}

static void red() {
  leds_fill(CRGB::Red);

  FastLED.show();
}

static void green() {
  leds_fill(CRGB::Green);
  
  FastLED.show();
}

static void blue() {
  leds_fill(CRGB::Blue);
  
  FastLED.show();
}

static void yellow() {
    leds_fill(CRGB::Yellow);
  
  FastLED.show();
}

static void orange() {
  leds_fill(CRGB::Orange);

  FastLED.show();
}

static void purple() {
  leds_fill(CRGB::Purple);

  FastLED.show();
}

static void pride() {
  static CRGB pride[] = { CRGB::Purple, CRGB::Indigo, CRGB::Turquoise, CRGB::Green, CRGB::Yellow, CRGB::Orange, CRGB::Red, CRGB::Pink };

  for(int i=0; i<NUM_LEDS; i++)
    leds[i] = pride[i % 8];

  FastLED.show();
}

static void xmas() {
  static CRGB xmas[2] = { CRGB::Red, CRGB::Green };

  for(int i=0; i<NUM_LEDS; i++)
    leds[i] = xmas[i % 2];

  FastLED.show();
}

static void rainbow() {
  fill_rainbow(&leds[0], NUM_LEDS, 0);

  FastLED.show();
}

static void independence() {
  static CRGB independence[3] = { CRGB::Red, CRGB::White, CRGB::Blue };

  for(int i=0; i<NUM_LEDS; i++)
    leds[i] = independence[i % 3];

  FastLED.show();
}

static void mango_parfait() {
  static CRGB parfait[2] = { CRGB::Orange, CRGB::White };

  for(int i=0; i<NUM_LEDS; i++)
    leds[i] = parfait[i % 2];

  FastLED.show();
}


static void reverse_watermelon() {
  static CRGB watermelon[4] = { CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Red };

  for(int i=0; i<NUM_LEDS; i++)
    leds[i] = watermelon[i % 4];

  FastLED.show();
}

static void random() {
  for(int i=0; i<NUM_LEDS; i++)
    leds[i] = CRGB(random8(), random8(), random8());

  FastLED.show();
}

static void off() {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = 0;

  FastLED.show();
}

preset_t presets[] = {
  "blue", blue,
  "green", green,
  "orange", orange,
  "pink", pink,
  "purple", purple,
  "red", red,
  "white", white,
  "yellow", yellow,
  "independence", independence,
  "mango parfait", mango_parfait,
  "pride", pride,
  "rainbow", rainbow,
  "random", random,
  "reverse watermelon", reverse_watermelon,
  "xmas", xmas,
  "off", off
};

unsigned presets_length = sizeof(presets)/sizeof(preset_t);
preset_t *current_preset;

void preset_set(preset_t *preset) {
  if(preset && preset->preset)
    (*preset->preset)();

  current_preset = preset;
}

preset_t* preset_lookup(const char* name) {
  for(int i = 0; i < sizeof(presets)/sizeof(preset); i++)
    if(strcmp(presets[i].name, name) == 0)
      return &presets[i];

  return NULL;
}


bool preset_set(const char* name) {
  preset_t *preset = preset_lookup(name);

  if(preset) {
    status_changed = true;

    animation_stop();
    leds_on();
    preset_set(preset);
    return true;
  }

  return false;
}

void preset_rgb(uint8_t red, uint8_t green, uint8_t blue) {
  status_changed = true;

  animation_stop();
  leds_on();
  leds_fill(CRGB(red, green, blue));
}
