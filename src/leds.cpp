#include "leds.h"
#include "animations.h"

CRGB leds[NUM_LEDS];

void leds_setup() {
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_CLOCK_PIN, LED_RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
}

void leds_handle() {
  static unsigned long led_update_time = 0;

  if(current_animation && millis() > led_update_time) {
    led_update_time = millis() + (*current_animation->animation)();
  }
}

