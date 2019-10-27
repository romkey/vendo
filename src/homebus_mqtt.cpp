#include "config.h"

#ifdef USE_MQTT

#include "multiball/app.h"
#include "multiball/wifi.h"
#include "multiball/mqtt.h"

#include "multiball/uptime.h"

#include "presets.h"
#include "animations.h"
#include "leds.h"
#include "homebus_mqtt.h"

#ifdef HAS_BME280
#include "multiball/bme280.h"
#endif

static Uptime uptime;

extern RTC_DATA_ATTR int bootCount;
extern RTC_DATA_ATTR int wifi_failures;

extern bool status_changed;

static String homebus_endpoint;

static void homebus_mqtt_start_announcement();
static void homebus_mqtt_publish_status();

void homebus_mqtt_setup() {
  homebus_endpoint = String("/homebus/device/") + MQTT_UUID;
  mqtt_subscribe((homebus_endpoint + "/cmd").c_str());

  homebus_mqtt_start_announcement();
}

void homebus_mqtt_handle() {
  bool should_publish = false;

#ifdef HAS_BME280
  static unsigned long next_update = 0;

  if(millis() > next_update) {
    should_publish = true;
    next_update = millis() + BME280_UPDATE_DELAY;
  }
#endif HAS_BME280

  if(status_changed) {
    should_publish = true;
    status_changed = false;
  }

  if(should_publish)
    homebus_mqtt_publish_status();
}

#define MAX_STATUS_LENGTH 512

static void homebus_mqtt_publish_status() {
  char buf[MAX_STATUS_LENGTH+1];

  snprintf(buf, MAX_STATUS_LENGTH, "{ \"id\": \"%s\", ", MQTT_UUID);
  snprintf(buf + strlen(buf),
	   MAX_STATUS_LENGTH - strlen(buf),
	   " \"system\": {  \"name\": \"%s\", \"build\": \"%s\", \"freeheap\": %d, \"uptime\": %lu, \"ip\": \"%s\", \"rssi\": %d, \"reboots\": %d, \"wifi_failures\": %d }, ",
	   App.hostname().c_str(), "", ESP.getFreeHeap(), uptime.uptime()/1000, App.ip_address().c_str(), WiFi.RSSI(), bootCount, wifi_failures);
#ifdef HAS_BME280
  snprintf(buf + strlen(buf),
	   MAX_STATUS_LENGTH - strlen(buf),
	   " \"environment\": { \"temperature\": %.1f, \"humidity\": %.1f, \"pressure\": %.1f }, ",
	   bme280_current_temperature(), bme280_current_humidity(), bme280_current_pressure());
#endif
  snprintf(buf + strlen(buf),
	   MAX_STATUS_LENGTH - strlen(buf),
	   " \"leds\": { \"number\": %u, \"status\": \"%s\"",
	   NUM_LEDS, leds_status() ? "on" : "off");

  if(leds_status())
    snprintf(buf + strlen(buf),
	     MAX_STATUS_LENGTH - strlen(buf),
	     ", \"preset\": \"%s\", \"animation\": \"%s\", \"speed\": %.2f, \"brightness\": %u } }",
	     current_preset ? current_preset->name : "none",
	     current_animation ? current_animation->name : "none",
	     animation_speed(), leds_brightness());
  else
    snprintf(buf + strlen(buf),
	     MAX_STATUS_LENGTH - strlen(buf),
	     " } }");

  Serial.println(buf);
  mqtt_publish(homebus_endpoint.c_str(), buf, true);
}

void homebus_mqtt_callback(const char* topic, char* command_buffer) {
  unsigned length = strlen(command_buffer);
  char* command = command_buffer;

  // command is meant to be a valid json string, so get rid of the quotes
  if(command[0] == '"' && command[length-1] == '"') {
    command[length-1] = '\0';
    command += 1;
  }

  Serial.printf("command %s\n", command);

  if(strcmp(command, "restart") == 0) {
    ESP.restart();
  }

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

    mqtt_publish("/leds/$error", command);
    return;
  }

  if(strncmp(command, "animation ", 10) == 0) {
    Serial.println("got animation");
    Serial.println(&command[10]);

    if(animation_set(&command[10]))
      return;

    mqtt_publish("/leds/$error", command);
    return;
  }
}

static void homebus_mqtt_start_announcement() {
#define TOPIC_NAME_LENGTH sizeof("/homebus/device/") + strlen(MQTT_UUID) + sizeof("/animations") + 1
  char topic_name[TOPIC_NAME_LENGTH];

  uint16_t animations_buf_size = sizeof("{ \"animations\": [ ] }") + 1;
  for(int i = 0; i < animations_length; i++)
    animations_buf_size += strlen(animations[i].name) + sizeof(", ");

  uint16_t presets_buf_size = sizeof("{ \"presets\": [ ] }") + 1;
  for(int i = 0; i < presets_length; i++)
    presets_buf_size += strlen(presets[i].name) + sizeof(" \"\", ");

  uint16_t buffer_length = max(animations_buf_size, presets_buf_size);
  char buf[buffer_length];

  snprintf(buf, buffer_length, "{ \"animations\": [ ");
  for(int i = 0; i < animations_length; i++) {
    uint16_t current_buf_len = strlen(buf);

    if(i > 0) {
      strncat(buf, ", ", current_buf_len);
      current_buf_len += 2;
    }

    snprintf(buf + current_buf_len, buffer_length - current_buf_len, "\"%s\"", animations[i].name);
  }

  strncat(buf, "]}", buffer_length - strlen(buf));

  snprintf(topic_name, TOPIC_NAME_LENGTH, "/homebus/device/%s/animations", MQTT_UUID);
  mqtt_publish(topic_name, buf, true);
  mqtt_publish("/status", buf, true);

  snprintf(buf, buffer_length, "{ \"presets\": [ ");
  for(int i = 0; i < presets_length; i++) {
    uint16_t current_buf_len = strlen(buf);

    if(i > 0) {
      strncat(buf, ", ", current_buf_len);
      current_buf_len += 2;
    }

    snprintf(buf + current_buf_len, buffer_length - current_buf_len, "\"%s\"", presets[i].name);
  }

  strncat(buf, "]}", buffer_length - strlen(buf));
  
  snprintf(topic_name, TOPIC_NAME_LENGTH, "/homebus/device/%s/presets", MQTT_UUID);
  mqtt_publish(topic_name, buf, true);
  mqtt_publish("/status", buf, true);
}



#endif // USE_MQTT
