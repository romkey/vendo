#include <Arduino.h>

#include "config.h"
#include "leds.h"
#include "presets.h"
#include "animations.h"
#include "hw.h"

CRGB leds[NUM_LEDS];

void setup() {
  delay(500);
  Serial.begin(115200);
  Serial.println("Hello World");

  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_CLOCK_PIN, LED_RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  while(1) {
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  leds[3] = CRGB::Red;
  leds[4] = CRGB::Pink;
  leds[5] = CRGB::Yellow;
  leds[6] = CRGB::Red;
  leds[7] = CRGB::Green;
  leds[8] = CRGB::Magenta;
  FastLED.show();
  delay(500);
  }

}

void loop() {
  
}
