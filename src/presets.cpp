#include "leds.h"
#include "presets.h"

static void red() {
}

static void green() {
}

static void blue() {
}

static void yellow() {
}

static void orange() {
}

static void purple() {
}

static void pride() {
}

preset_t presets[] = {
  "red", red,
  "green", green,
  "blue", blue,
  "yellow", yellow,
  "orange", orange,
  "purple", purple,
  "pride", pride
};

void preset_set(preset_t *preset) {
}

bool preset_set(const char* name) {
  return true;
}

