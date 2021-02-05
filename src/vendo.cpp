#include <ArduinoJson.h>

#include "config.h"

#include "multiball/app.h"
#include "multiball/wifi.h"
#include "multiball/homebus.h"

#include "multiball/uptime.h"

#include "presets.h"
#include "animations.h"
#include "leds.h"
#include "vendo.h"

#include "cmd_json.h"


#ifdef HAS_BME280
#include "multiball/bme280.h"
#endif

static void vendo_start_announcement();
static void vendo_publish_status();

void vendo_setup() {
  vendo_start_announcement();
}

void vendo_handle() {
  bool should_publish = false;

#ifdef HAS_BME280
  static unsigned long next_update = 0;

  if(millis() > next_update) {
    should_publish = true;
    next_update = millis() + BME280_UPDATE_DELAY;
  }
#endif // HAS_BME280

  if(App.updates_available()) {
    should_publish = true;
    App.updates_available(false);
  }

  if(should_publish)
    vendo_publish_status();
}

#define MAX_STATUS_LENGTH 512

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

/*
{
  "org.homebus.leds": {
    "number": 5,
    "controller": "WS2812B",
    "status": "on",
    "preset": "red",
    "animation": {
      "name": "throb",
      "speed": 2.3
    },
    "brightness": 100,
    "rgb": {
      "red": 252,
      "green": 100,
      "blue": 50
    },
    "gradient": {
      "start": {
        "red": 252,
        "green": 100,
        "blue": 50
      },
      "end": {
        "red": 252,
        "green": 100,
        "blue": 50
      }
    }
  }
}
*/

boolean vendo_led_status(char *buf, size_t buflen) {
  snprintf(buf, buflen,
	   "{ \"status\": \"%s\", \"preset\": \"%s\", \"animation\": \"%s\", \"speed\": %.2f, \"brightness\": %u, \"rgb\": { \"red\": %d, \"green\": %d, \"blue\": %d } }",
	   leds_status() ? "on" : "off", 
	   current_preset ? current_preset->name : "none",
	   current_animation ? current_animation->name : "none",
	   animation_speed(), leds_brightness(),
	   leds[0].red, leds[0].green, leds[0].blue);

  Serial.println(buf);

  return true;
}

static boolean vendo_led_config(char *buf, size_t buflen) {
  static boolean published = false;

  if(published)
    return false;

  snprintf(buf, buflen,
	   "{ \"led-count\": %u, \"controller\": \"%s\" }",
	   NUM_LEDS, STRINGIZE(LED_TYPE));

  return true;
}

#ifdef HAS_BME280
static boolean vendo_led_air(char *buf, size_t buflen) {
  snprintf(buf,
	   buflen,
	   "{  \"temperature\": %.1f, \"humidity\": %.1f, \"pressure\": %.1f }",
	   bme280_current_temperature(), bme280_current_humidity(), bme280_current_pressure());

  Serial.println(buf);

  published = true;

  return true;
}
#endif

static boolean vendo_led_system(char *buf, size_t buflen) {
  static IPAddress oldIP = IPAddress(0, 0, 0, 0);
  static String mac_address = WiFi.macAddress();
  IPAddress local = WiFi.localIP();

  if(oldIP == local)
    return false;

  snprintf(buf,
	   buflen,
	   "{ \"name\": \"%s\", \"platform\": \"%s\", \"build\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"mac_addr\": \"%s\" }",
	   App.hostname().c_str(), "discoball", App.build_info().c_str(), local[0], local[1], local[2], local[3], mac_address.c_str());

  Serial.println(buf);

  return true;
}

static boolean vendo_led_diagnostic(char *buf, size_t buflen) {
    snprintf(buf,
	     buflen,
	     "{ \"platform\": \"%s\", \"freeheap\": %d, \"uptime\": %lu, \"rssi\": %d, \"reboots\": %d, \"wifi_failures\": %d }",
	     "discoball", ESP.getFreeHeap(), App.uptime()/1000, WiFi.RSSI(), App.boot_count(), App.wifi_failures());

    return true;
}

static void vendo_publish_status() {
  char buf[MAX_STATUS_LENGTH];

  if(vendo_led_status(buf, MAX_STATUS_LENGTH)) 
    homebus_publish_to(DDC_LED_UPDATE, buf);

  if(vendo_led_config(buf, MAX_STATUS_LENGTH))
    homebus_publish_to(DDC_LED_CONFIG, buf);

  if(vendo_led_system(buf, MAX_STATUS_LENGTH))
    homebus_publish_to(DDC_SYSTEM, buf);
 
  if(vendo_led_diagnostic(buf, MAX_STATUS_LENGTH))
    homebus_publish_to(DDC_DIAGNOSTIC, buf);

#ifdef HAS_BME280
  if(vendo_led_air(buf, MAX_STATUS_LENGTH))
    homebus_publish_to(DDC_AIR_SENSOR, buf);
#endif
}

void vendo_callback(const char* topic, char* command_buffer) {
  cmd_json(command_buffer);
}


static uint16_t vendo_animations_json_length() {
  uint16_t animations_buf_size = sizeof("[ ]") + 1;
  for(int i = 0; i < animations_length; i++)
    animations_buf_size += strlen(animations[i].name) + sizeof(", ");

  return animations_buf_size;
}

static uint16_t vendo_presets_json_length() {
  uint16_t presets_buf_size = sizeof("[ ]") + 1;
  for(int i = 0; i < presets_length; i++)
    presets_buf_size += strlen(presets[i].name) + sizeof(" \"\", ");

  return presets_buf_size;
}

static void vendo_get_animations_json(char *buf, uint16_t buffer_length) {
  snprintf(buf, buffer_length, "[ ");
  for(int i = 0; i < animations_length; i++) {
    uint16_t current_buf_len = strlen(buf);

    if(i > 0) {
      strncat(buf, ", ", current_buf_len);
      current_buf_len += 2;
    }

    snprintf(buf + current_buf_len, buffer_length - current_buf_len, "\"%s\"", animations[i].name);
  }

  strncat(buf, "]", buffer_length - strlen(buf));
}

static void vendo_get_presets_json(char *buf, uint16_t buffer_length) {
  snprintf(buf, buffer_length, "[ ");
  for(int i = 0; i < presets_length; i++) {
    uint16_t current_buf_len = strlen(buf);

    if(i > 0) {
      strncat(buf, ", ", current_buf_len);
      current_buf_len += 2;
    }

    snprintf(buf + current_buf_len, buffer_length - current_buf_len, "\"%s\"", presets[i].name);
  }

  strncat(buf, "]", buffer_length - strlen(buf));
}

static void vendo_start_announcement() {
  uint16_t animations_length = vendo_animations_json_length();
  uint16_t presets_length = vendo_presets_json_length();

  uint16_t buffer_length = max(animations_length, presets_length);
  Serial.printf("presets %d animations %d length %d\n", presets_length, animations_length, buffer_length);
  delay(500);

  char buf[buffer_length + 1];

  vendo_get_animations_json(buf, buffer_length);
  Serial.println(buf);
  homebus_publish_to(DDC_LED_ANIMATIONS, buf);
  
  vendo_get_presets_json(buf, buffer_length);
  Serial.println(buf);
  homebus_publish_to(DDC_LED_PRESETS, buf);
}
