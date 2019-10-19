#include <multiball/app.h>

#include "config.h"
#include "leds.h"
#include "animations.h"

#include <SPIFFS.h>
#include <FS.h>

CRGB leds[NUM_LEDS];

void leds_setup() {

#ifdef LED_CLOCK_PIN
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_CLOCK_PIN, LED_RGB>(leds, NUM_LEDS);
#else
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_RGB>(leds, NUM_LEDS);
#endif

  FastLED.setCorrection(TypicalLEDStrip);

  leds_restore();
}

void leds_handle() {
  static unsigned long led_update_time = 0;

  if(current_animation && millis() > led_update_time) {
    led_update_time = millis() + animate();
  }
}

static bool leds_state = false;

bool leds_status() {
  return leds_state;
}

void leds_on() {
  App.updates_available(true);

  leds_state = true;
}

void leds_off() {
  App.updates_available(true);

  leds_state = false;

  animation_stop();
  leds_fill(CRGB(0,0,0));
  FastLED.show();
}

static uint8_t stored_brightness = 100;
static uint8_t maximum_brightness = 100;

uint8_t leds_brightness(void) {
  return stored_brightness;
}

void leds_brightness(uint8_t brightness) {
  if(brightness > maximum_brightness)
    brightness = maximum_brightness;

  if(brightness == stored_brightness)
    return;

  App.updates_available(true);

  stored_brightness = brightness;

  FastLED.setBrightness(map(brightness, 0, 100, 0, 255));
  FastLED.show();
}

uint8_t leds_maximum_brightness(void) {
  return maximum_brightness;
}

#define LEDS_MAXIMUM_BRIGHTNESS_PERSISTENCE_FILE "/config/leds_maximum_brightness"

void leds_maximum_brightness(uint8_t brightness) {
  if(brightness == 0)
    brightness = 1;
  if(brightness > 100)
    brightness = 100;

  maximum_brightness = brightness;
  if(stored_brightness > brightness)
    leds_brightness(brightness);

  // always persist maximum brightness
  File file = SPIFFS.open(LEDS_MAXIMUM_BRIGHTNESS_PERSISTENCE_FILE, FILE_WRITE);
  if(file) {
    file.println(brightness);
    file.close();
  }
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

#define LEDS_PERSISTENCE_FILE "/config/leds"
#define LEDS_BRIGHTNESS_PERSISTENCE_FILE "/config/leds_brightness"

void leds_clear_persist() {
  App.config.clear("leds", "");
  App.config.clear("leds", "brightness");
}

// don't even save state if LEDs are on
void leds_persist() {
  App.config.set("leds", "brightness", String(stored_brightness));
}


void leds_restore() {
  boolean success = false;
  String results;

  results = App.config.get("leds", "brightness", &success);
  if(success)
    leds_brightness(atoi(results.c_str()));

  results = App.config.get("leds", "maximum_brightness", &success);
  if(success)
    leds_maximum_brightness(atoi(results.c_str()));




=======
  File file = SPIFFS.open(LEDS_PERSISTENCE_FILE, FILE_READ);
  if(file) {
    leds_off();
    file.close();
  }

  file =  SPIFFS.open(LEDS_BRIGHTNESS_PERSISTENCE_FILE, FILE_READ);
  if(file) {
    char buffer[32];
    while(file.available()) {
      int length = file.readBytesUntil('\n', buffer, sizeof(buffer));
      buffer[length > 0 ? length - 1 : 0] =  '\0';
    }
    leds_brightness(atoi(buffer));
    file.close();
  }

  file =  SPIFFS.open(LEDS_MAXIMUM_BRIGHTNESS_PERSISTENCE_FILE, FILE_READ);
  if(file) {
    char buffer[32];
    while(file.available()) {
      int length = file.readBytesUntil('\n', buffer, sizeof(buffer));
      buffer[length > 0 ? length - 1 : 0] =  '\0';
    }
    leds_maximum_brightness(atoi(buffer));
    file.close();
  }
>>>>>>> a1ca377b090cfe2b3f37d7e466f47e5602fab48c
}
