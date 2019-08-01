#include "config.h"
#include "leds.h"
#include "animations.h"

unsigned animation_throb(bool init) {
  static uint8_t brightness = 255;
  static int8_t direction = -1;
  static CRGB old_leds[NUM_LEDS];

  if(init) {
    for(int i = 0; i < NUM_LEDS; i++)
      old_leds[i] = leds[i];

    return 0;
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
