#include "leds.h"
#include "animations.h"

CRGB leds[NUM_LEDS];

void leds_setup() {
#ifdef LED_CLOCK_PIN
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_CLOCK_PIN, LED_RGB>(leds, NUM_LEDS);
#else
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_RGB>(leds, NUM_LEDS);
#endif
  FastLED.setBrightness(100);
}

void leds_handle() {
  static unsigned long led_update_time = 0;

  if(current_animation && millis() > led_update_time) {
    led_update_time = millis() + animate();
  }
}

void leds_on() {
}

void leds_off() {
}

void leds_brightness(uint8_t brightness) {
  if(brightness > 100)
    brightness = 100;

  FastLED.setBrightness(map(brightness, 0, 100, 0, 256));
}

void leds_fill(uint8_t red, uint8_t green, uint8_t blue) {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB(red, green, blue);

  FastLED.show();
}

void leds_fill(CRGB color) {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = color;

  FastLED.show();
}

