#include "leds.h"
#include "presets.h"

static void fill_leds(CRGB color) {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = color;
  FastLED.show();
}

static void pink() {
  fill_leds(CRGB::Pink);
}

static void red() {
  fill_leds(CRGB::Red);
}

static void green() {
  fill_leds(CRGB::Green);
}

static void blue() {
  fill_leds(CRGB::Blue);
}

static void yellow() {
    fill_leds(CRGB::Yellow);
}

static void orange() {
    fill_leds(CRGB::Orange);
}

static void purple() {
    fill_leds(CRGB::Purple);
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


preset_t presets[] = {
  "red", red,
  "green", green,
  "blue", blue,
  "yellow", yellow,
  "orange", orange,
  "purple", purple,
  "pink", pink,
  "pride", pride,
  "xmas", xmas
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
    preset_set(preset);
    return true;
  }

  return false;
}

