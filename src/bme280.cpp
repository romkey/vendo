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

void bme280_setup() {
  bme280.begin();
}

static unsigned long next_read = 0;

void bme280_handle() {
  if(millis() < next_read)
    return;

#define BUFFER_SIZE 500
  char buf[BUFFER_SIZE];
  IPAddress local = WiFi.localIP();

  extern char *build_info, *hostname;

  snprintf(buf, BUFFER_SIZE, "{ \"id\": %s, \"system\": {\"name\": \"%s\", \"build\": \"%s\", \"freeheap\": %d, \"uptime\": %lu, \"ip\": \"%d.%d.%d.%d\", \"rssi\": %d }, \"environment\": { \"temperature\": %.1f, \"humidity\": %.1f, \"pressure\": %.1f } }",
	   MQTT_UUID,
	   hostname, build_info, ESP.getFreeHeap(), uptime.uptime()/1000, local[0], local[1], local[2], local[3], WiFi.RSSI(),
	   bme280.readTemperature(), bme280.readHumidity(), bme280.readPressure());	   

  mqtt_publish("/vendo", buf);

  next_read = millis() + BME280_UPDATE_DELAY;
}
#endif
