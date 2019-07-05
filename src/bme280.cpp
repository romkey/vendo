#include <Arduino.h>

#include <WiFi.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "mqtt.h"
#include "bme280.h"
#include "uptime.h"

#include "config.h"

#ifdef HAS_BME280

static Adafruit_BME280 bme280;
static Uptime uptime;

#define MAX_UUID_LENGTH 37
#define HOMEBUS_ENDPOINT_LENGTH sizeof("/homebus/device/") + MAX_UUID_LENGTH
static char homebus_endpoint[HOMEBUS_ENDPOINT_LENGTH + 1];

void bme280_setup() {
  if(!bme280.begin(0x76))
    Serial.println("BME280 not found");
  else
    Serial.println("[bme280]");

  snprintf(homebus_endpoint, HOMEBUS_ENDPOINT_LENGTH, "/homebus/device/%s", MQTT_UUID);
  Serial.printf("Homebus endpoint: %s\n", homebus_endpoint);
}

static unsigned long next_read = 0;

extern char *build_info, *hostname;

void bme280_handle() {
  if(millis() < next_read)
    return;

#define BUFFER_SIZE 500
  char buf[BUFFER_SIZE];
  IPAddress local = WiFi.localIP();

  snprintf(buf, BUFFER_SIZE, "{ \"id\": \"%s\", \"system\": {\"name\": \"%s\", \"build\": \"%s\", \"freeheap\": %d, \"uptime\": %lu, \"ip\": \"%d.%d.%d.%d\", \"rssi\": %d }, \"environment\": { \"temperature\": %.1f, \"humidity\": %.1f, \"pressure\": %.1f } }",
	   MQTT_UUID,
	   //	   hostname, build_info, ESP.getFreeHeap(), uptime.uptime()/1000, local[0], local[1], local[2], local[3], WiFi.RSSI(),
	   "", "", ESP.getFreeHeap(), uptime.uptime()/1000, local[0], local[1], local[2], local[3], WiFi.RSSI(),
	   bme280.readTemperature(), bme280.readHumidity(), bme280.readPressure());	   

#ifdef VERBOSE
  Serial.println(buf);
#endif

  mqtt_publish(BME280_MQTT_PUBLISH_TOPIC, buf);
  mqtt_publish(homebus_endpoint, buf);

  next_read = millis() + BME280_UPDATE_DELAY;
}
#endif
