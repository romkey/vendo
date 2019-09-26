#pragma once

#include "leds.h"

typedef struct preset {
  const char* name;
  void (*preset)();  } preset_t;

extern preset_t presets[];
extern unsigned presets_length;
extern preset_t* current_preset;

void preset_setup();

void preset_rgb(uint8_t red, uint8_t green, uint8_t blue);
void preset_set(preset_t*);
bool preset_set(const char*);

void preset_persist();
void preset_clear_persist();
void preset_restore();
