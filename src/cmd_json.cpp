#include <Esp.h>
#include <ArduinoJson.h>

#include "config.h"

#include <multiball/app.h>
#include <multiball/homebus.h>


#include "leds.h"
#include "presets.h"
#include "animations.h"

#include "cmd_json.h"

/*
  process a command expressed in JSON
  {
    "status": "off",
    "preset" : "reverse watermelon",
    "animation": "lightning",
    "speed": 5.5,
    "brightness": 80,
    "rgb": { "red": 255, "green": 255, "blue": 255 }
  }

  https://arduinojson.org/v6/assistant/

*/


void cmd_json(const char *json) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, json);

  if(err) {
    Serial.println("deserialization error on JSON command");
    Serial.println(err.c_str());
    Serial.println(json);
    return;
  }

  //  Serial.println("successful JSON deserialization");

  const char* status = doc["status"];
  if(status != nullptr) {
    if(strcmp(status, "on") == 0) {
      leds_on();
    } else if(strcmp(status, "off") == 0) {
      leds_off();
    } else if(strcmp(status, "restart") == 0) {
      ESP.restart();
    } else if(strcmp(status, "persist") == 0) {
      App.persist();
    } else if(strcmp(status, "clear-persist") == 0) {
      leds_clear_persist();
      preset_clear_persist();
      animation_clear_persist();
    } else if(strcmp(status, "clear-homebus") == 0) {
      homebus_reset();
    }
  }

  const char* preset = doc["preset"];
  if(preset != nullptr) {
    preset_set(preset);
  }

  const char* animation = doc["animation"];
  if(animation != nullptr) {
    animation_set(animation);
  }

  if(doc.containsKey("speed")) {
    float speed = doc["speed"];

    animation_speed(speed);
  }

  if(doc.containsKey("brightness")) {
    int brightness = doc["brightness"];

    leds_brightness(brightness);
  }

  if(doc.containsKey("rgb")) {
    JsonObject rgb = doc["rgb"];
    if(rgb.containsKey("red") && rgb.containsKey("green") && rgb.containsKey("blue")) {
      int rgb_red = rgb["red"];
      int rgb_green = rgb["green"];
      int rgb_blue = rgb["blue"];

      //      Serial.printf("rgb %d, %d, %d\n", rgb_red, rgb_green, rgb_blue);

      preset_rgb(rgb_red, rgb_green, rgb_blue);
    }
  }
}
