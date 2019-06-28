#include "leds.h"
#include "animations.h"
#include "presets.h"

static void white() {
  leds_fill(CRGB::White);
}

static void pink() {
  leds_fill(CRGB::Pink);
}

static void red() {
  leds_fill(CRGB::Red);
}

static void green() {
  leds_fill(CRGB::Green);
}

static void blue() {
  leds_fill(CRGB::Blue);
}

static void yellow() {
    leds_fill(CRGB::Yellow);
}

static void orange() {
    leds_fill(CRGB::Orange);
}

static void purple() {
    leds_fill(CRGB::Purple);
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

static void off() {
  for(int i = 0; i < NUM_LEDS; i++)
	leds[0] = 0;

  FastLED.show();
}

preset_t presets[] = {
  "white", white,
  "red", red,
  "green", green,
  "blue", blue,
  "yellow", yellow,
  "orange", orange,
  "purple", purple,
  "pink", pink,
  "pride", pride,
  "xmas", xmas,
  "rainbow", rainbow,
  "off", off
};

unsigned presets_length = sizeof(presets)/sizeof(preset_t);

void preset_set(preset_t *preset) {
  if(preset && preset->preset)
    (*preset->preset)();
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
    animation_stop();
    preset_set(preset);
    return true;
  }

  return false;
}

