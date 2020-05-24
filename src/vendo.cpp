#include "config.h"

#include "multiball/app.h"
#include "multiball/wifi.h"
#include "multiball/homebus.h"

#include "multiball/uptime.h"

#include "presets.h"
#include "animations.h"
#include "leds.h"
#include "vendo.h"

#include <ArduinoJson.h>


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
#endif HAS_BME280

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

void vendo_led_status(char *buf, size_t buflen) {
  snprintf(buf, buflen,
	   "{ \"id\": \"%s\", \"org.homebus.experimental.led-control\": { \"status\": \"%s\", \"preset\": \"%s\", \"animation\": \"%s\", \"speed\": %.2f, \"brightness\": %u } }",
	   homebus_uuid().c_str(),
	   leds_status() ? "on" : "off", 
	   current_preset ? current_preset->name : "none",
	   current_animation ? current_animation->name : "none",
	   animation_speed(), leds_brightness());
}

static void vendo_led_config(char *buf, size_t buflen) {
  snprintf(buf, buflen,
	   "{ \"id\": \"%s\", \"org.homebus.experimental.led-config\": { \"led-count\": %u, \"controller\": \"%s\" } }",
	   homebus_uuid().c_str(),
	   NUM_LEDS, STRINGIZE(LED_TYPE));
}

#ifdef HAS_BME280
static void vendo_led_air(char *buf, size_t buflen) {
  snprintf(buf, buflen,
	   "{ \"id\": \"%s\", \"org.homebus.experimental.air-sensor\": {  \"temperature\": %.1f, \"humidity\": %.1f, \"pressure\": %.1f }, ",
	   homebus_uuid().c_str(),
	   bme280_current_temperature(), bme280_current_humidity(), bme280_current_pressure());
}
#endif

static void vendo_led_system(char *buf, size_t buflen) {
  static IPAddress oldIP = IPAddress(0, 0, 0, 0);
  static String mac_address = WiFi.macAddress();
  IPAddress local = WiFi.localIP();

  snprintf(buf, buflen,
	   "{ \"id\": \"%s\", \"org.homebus.experimental.led-system\": { \"name\": \"%s\", \"build\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"mac_addr\": \"%s\" } }",
	   homebus_uuid().c_str(),
	   App.hostname().c_str(), App.build_info().c_str(), local[0], local[1], local[2], local[3], mac_address.c_str());
}

static void vendo_led_diagnostic(char *buf, size_t buflen) {
    snprintf(buf, buflen, "{ \"id\": \"%s\", \"org.homebus.experimental.led-diagnostic\": { \"freeheap\": %d, \"uptime\": %lu, \"rssi\": %d, \"reboots\": %d, \"wifi_failures\": %d } }",
	   homebus_uuid().c_str(),
	   ESP.getFreeHeap(), App.uptime()/1000, WiFi.RSSI(), App.boot_count(), App.wifi_failures());
}

static void vendo_publish_status() {
  char buf[MAX_STATUS_LENGTH];

  vendo_led_status(buf, MAX_STATUS_LENGTH);
  homebus_publish_to(buf, "org.homebus.experimental.led-update");

  vendo_led_config(buf, MAX_STATUS_LENGTH);
  homebus_publish_to(buf, "org.homebus.experimental.led-config");

  vendo_led_system(buf, MAX_STATUS_LENGTH);
  homebus_publish_to(buf, "org.homebus.experimental.led-system");
 
  vendo_led_diagnostic(buf, MAX_STATUS_LENGTH);
  homebus_publish_to(buf, "org.homebus.experimental.led-diagnostic");

#ifdef HAS_BME280
  vendo_led_air(buf, MAX_STATUS_LENGTH);
  homebus_publish_to(buf, "org.homebus.experimental.air-sensor");
#endif
}

void vendo_callback(const char* topic, char* command_buffer) {
  unsigned length = strlen(command_buffer);
  char* command = command_buffer;

  // command is meant to be a valid json string, so get rid of the quotes
  if(command[0] == '"' && command[length-1] == '"') {
    command[length-1] = '\0';
    command += 1;
  }

  Serial.printf("command %s\n", command);

  if(strcmp(command, "off") == 0) {
    leds_off();
    return;
  }

  if(strcmp(command, "stop") == 0) {
    animation_stop();
    return;
  }

  if(strncmp(command, "speed ", 6) == 0) {
    Serial.printf("speed %s\n", &command[6]);
    animation_speed(String(&command[6]).toFloat());
    return;
  }

  if(strncmp(command, "bright ", 7) == 0) {
    leds_brightness(atoi(&command[7]));
    return;
  }

  if(strncmp(command, "rgb ", 4) == 0) {
    char temp[3] = "xx";
    uint8_t red, green, blue;

    Serial.printf("RGB %s\n", &command[4]);
    temp[0] = command[4];
    temp[1] = command[5];
    red = strtol(temp, 0, 16);

    temp[0] = command[6];
    temp[1] = command[7];
    green = strtol(temp, 0, 16);

    temp[0] = command[8];
    temp[1] = command[9];
    blue = strtol(temp, 0, 16);

    Serial.printf("rgb red %u, green %u, blue %u\n", red, green, blue);
    preset_rgb(red, green, blue);
  }

  if(strncmp(command, "preset ", 7) == 0) {
    Serial.println("got preset");
    Serial.println(&command[7]);

    if(preset_set(&command[7]))
      return;

    homebus_publish_to("$error", command);
    return;
  }

  if(strncmp(command, "animation ", 10) == 0) {
    Serial.println("got animation");
    Serial.println(&command[10]);

    if(animation_set(&command[10]))
      return;

    homebus_publish_to("$error", command);
    return;
  }
}

static void vendo_start_announcement() {
  uint16_t animations_buf_size = sizeof("[ ]") + 1;
  for(int i = 0; i < animations_length; i++)
    animations_buf_size += strlen(animations[i].name) + sizeof(", ");

  uint16_t presets_buf_size = sizeof("[ ]") + 1;
  for(int i = 0; i < presets_length; i++)
    presets_buf_size += strlen(presets[i].name) + sizeof(" \"\", ");

  uint16_t buffer_length = max(animations_buf_size, presets_buf_size);
  char buf[buffer_length];

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

  homebus_publish_to("animations", buf);

  snprintf(buf, buffer_length, "[ ");
  for(int i = 0; i < presets_length; i++) {
    uint16_t current_buf_len = strlen(buf);

    if(i > 0) {
      strncat(buf, ", ", current_buf_len);
      current_buf_len += 2;
    }

    snprintf(buf + current_buf_len, buffer_length - current_buf_len, "\"%s\"", presets[i].name);
  }

  strncat(buf, "}", buffer_length - strlen(buf));
  
  homebus_publish_to("presets", buf);
}
