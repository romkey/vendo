#include <SPIFFS.h>
#include <FS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "config.h"

#include "http_server.h"

#include "leds.h"
#include "presets.h"
#include "animations.h"

#include "multiball/bme280.h"
#include "multiball/app.h"
#include "multiball/wifi.h"
#include "multiball/mqtt.h"
#include "multiball/homebus.h"

#include "vendo.h"

// static WiFiClient client;
static AsyncWebServer server(80);

// most web requests are handled from the root page
// requests look like:
//  /?r=XXX&g=XXX&b=XXX
//  /?preset=NAME
//  /?sequence=NAME or /?animation=NAME
//  /?brightness=XXX
//  /?maximum_brightness=XXX
//  /?speed=XXX
// /persist
// /on
// /off
// /start
// /stop
// /status.json

static void handle_root(AsyncWebServerRequest *request), handle_on(AsyncWebServerRequest *request), handle_off(AsyncWebServerRequest *request), handle_start(AsyncWebServerRequest *request), handle_stop(AsyncWebServerRequest *request), handle_status(AsyncWebServerRequest *request), handle_persist(AsyncWebServerRequest *request), handle_clear_persist(AsyncWebServerRequest *request);

// set up the web server
// register the URLs that it handles, and set things up to serve files from flash storage
// SPIFFs filenames are limited to 31 characters, so we'll try to keep pathnames short
void http_server_setup() {
  server.on("/", HTTP_GET, handle_root);
  server.on("/", HTTP_POST, handle_root);
  server.on("/persist", HTTP_GET, handle_persist);
  server.on("/clear_persist", HTTP_GET, handle_clear_persist);
  server.on("/off", HTTP_GET, handle_off);
  server.on("/stop", HTTP_GET, handle_stop);
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) { ESP.restart(); });
  server.on("/status.json", HTTP_GET, handle_status);

  server.serveStatic("/static", SPIFFS, "/w/s/");

  server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(404, "text/plain", "File not found");
    });

  server.begin();
}

// not really needed anymore but we'll keep it around just in case
void http_server_handle() {
}

// called for each string we might replace
static String template_handler(const String &var) {
  if(var == "HOSTNAME")
    return App.hostname();

  if(var == "LEDS_BRIGHTNESS")
    return String(leds_brightness());

  if(var == "ANIMATION_SPEED")
    return String(animation_speed());

  if(var == "MAXIMUM_BRIGHTNESS") {
    if(leds_maximum_brightness() == 100)
      return String("");

    return String(" (maximum brightness ") + String(leds_maximum_brightness()) + "percent)";
  }

  if(var == "MAC_ADDRESS")
    return App.mac_address();

  if(var == "IP_ADDRESS")
    return App.ip_address();

  if(var == "SSID")
    return WiFi.SSID();

  if(var == "RSSI")
    return String(WiFi.RSSI());

  if(var == "NUM_LEDS")
    return String(NUM_LEDS);

  if(var == "LED_DATA_PIN")
    return String(LED_DATA_PIN);

  if(var == "LED_CLOCK_PIN")
#ifdef LED_CLOCK_PIN
    return String(LED_CLOCK_PIN);
#else
  return String("unused");
#endif


#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
  if(var == "LED_CONTROLLER")
    return String(STRINGIZE(LED_TYPE));

  if(var == "LED_RGB")
    return String(STRINGIZE(LED_RGB));

  if(var == "SUBNET_MASK")
    return String(WiFi.subnetMask()[0]) + "."  + String(WiFi.subnetMask()[1]) + "."  + String(WiFi.subnetMask()[2]) + "."  + String(WiFi.subnetMask()[3]);

  if(var == "DEFAULT_ROUTER")
    return String(WiFi.gatewayIP()[0]) + "."  + String(WiFi.gatewayIP()[1]) + "."  + String(WiFi.gatewayIP()[2]) + "."  + String(WiFi.gatewayIP()[3]);

  if(var == "PRESET_OPTIONS") {
    String result = "";

    if(current_preset == NULL)
      result += "  <option value='' selected disabled hidden>none selected</option>";

    for(int i = 0; i < presets_length; i++) {
      result += "  <option value='";
      result += presets[i].name;
      if(&presets[i] == current_preset)
	result += "' selected>";
      else
	result += "'>";

      result += presets[i].name;
      result += "</option>";
    }

  return result;
  }

  if(var == "ANIMATION_OPTIONS") {
    String result = "";

    if(current_animation == NULL)
      result += "  <option value='' selected disabled hidden>none selected</option>";

    bool any_animation_ignores_presets = false;
    for(int i = 0; i < animations_length; i++) {
      result += "  <option value='";
      result += animations[i].name;
      if(&animations[i] == current_animation)
	result += "' selected>";
      else
	result += "'>";

      result += animations[i].name;
      if(animations[i].ignores_presets) {
	any_animation_ignores_presets = true;
	result += " *";
      }
      result += "</option>";
    }

    return result;
  }

  if(var == "FILE_LIST") {
    String result = "";

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
 
    while(file) {
      result += " " + String(file.name()) + " ";
      file.close();
      file = root.openNextFile();
    }

    root.close();
    return result;
  }

  if(var == "SPIFFS_USED_BYTES")
    return String(SPIFFS.usedBytes());

  if(var == "SPIFFS_TOTAL_BYTES")
    return String(SPIFFS.totalBytes());

  if(var == "MQTT_INFO") {
    String result = "";
    
    result = String("MQTT (") + (mqtt_is_connected() ? "" : "not ") + "connected)broker:</b> " + homebus_mqtt_host() + " <b>port:</b> " + String(homebus_mqtt_port()) + " <b>username:</b> " + homebus_mqtt_username() + " <b>UUID:</b> " + homebus_mqtt_uuid();

    return result;
  }

  if(var == "ENVIRONMENT") {
    String result;

#ifdef HAS_BME280
    result = "<div class='row'>"
      "<div class='card col-sm border-dark border-rounded'>"
      "<div class='card-body'>"
      "<h2 class='card-title'>Environment</h2>"
      "<b>Temperature:</b> " + String(bme280_current_temperature()) + "°C"
      "<br/><b>Pressure:</b> " + String(bme280_current_pressure()) +
      "<br/><b>Humidity:</b> " + String(bme280_current_humidity()) + " percent"
      "</div>"
      "</div>"
      "</div><!- row ->";
#endif

    return result;
  }

  if(var == "FREEHEAP")
    return String(ESP.getFreeHeap());

  if(var == "UPTIME")
    return String(App.uptime());

  return var;
}

static void handle_root(AsyncWebServerRequest *request) {
  if(request->hasArg("hostname")) {
    App.hostname(request->arg("hostname"));
    App.persist();
  }

  if(request->hasArg("preset")) {
    preset_set(request->arg("preset").c_str());
  } 

  if(request->hasArg("animation"))
    animation_set(request->arg("animation").c_str());
  if(request->hasArg("sequence"))
    animation_set(request->arg("sequence").c_str());

  if(request->hasArg("r") && request->hasArg("g") && request->hasArg("b")) {
    uint8_t red, green, blue;

    red = strtol(request->arg("r").c_str(), 0, 16);
    green = strtol(request->arg("g").c_str(), 0, 16);
    blue = strtol(request->arg("b").c_str(), 0, 16);

    preset_rgb(red, green, blue);
  }

  if(request->hasArg("brightness"))
    leds_brightness(request->arg("brightness").toInt());

  if(request->hasArg("maximum_brightness"))
    leds_maximum_brightness(request->arg("maximum_brightness").toInt());

  if(request->hasArg("speed")) {
    float speed = request->arg("speed").toFloat();

    if(speed)
      animation_speed(speed);
  }

  if(request->method() == HTTP_GET) {
    request->send(SPIFFS, "/w/index.html", String(), false, template_handler);
    return;
  }

 char buf[512];
 vendo_led_status(buf, 512);

 request->send(200, "application/json", buf);
}

static void handle_off(AsyncWebServerRequest *request) {
  preset_set("off");
  leds_off();
  current_preset = NULL;
  current_animation = NULL;
  animation_speed(1);

  request->redirect("/");
}

static void handle_start(AsyncWebServerRequest *request) {
  animation_start();

  request->redirect("/");
}

static void handle_stop(AsyncWebServerRequest *request) {
  animation_stop();

  request->redirect("/");
}

static void handle_persist(AsyncWebServerRequest *request) {
  leds_persist();
  preset_persist();
  animation_persist();

  request->redirect("/");
}

static void handle_clear_persist(AsyncWebServerRequest *request) {
  leds_clear_persist();
  preset_clear_persist();
  animation_clear_persist();

  request->redirect("/");
}

static void handle_status(AsyncWebServerRequest *request) {
}
