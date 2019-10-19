#include <SPIFFS.h>
#include <FS.h>

#include "config.h"

#include "http_server.h"

#include "leds.h"
#include "presets.h"
#include "animations.h"

#include "multiball/bme280.h"
#include "multiball/wifi.h"
#include "multiball/mqtt.h"

#ifdef ESP8266
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include <WiFiClient.h>

static WiFiClient client;

#ifdef ESP8266
static ESP8266WebServer server(80);
#else
static WebServer server(80);
#endif

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

static void handle_root(), handle_on(), handle_off(), handle_start(), handle_stop(), handle_status(), handle_persist(), handle_clear_persist();

void http_server_setup() {
  server.on("/", handle_root);
  server.on("/persist", handle_persist);
  server.on("/clear_persist", handle_clear_persist);
  server.on("/off", handle_off);
  server.on("/stop", handle_stop);
  server.on("/restart", []() { ESP.restart(); });
  server.on("/status.json", handle_status);

  server.onNotFound([]() {
      server.send(404, "text/plain", "File not found");
    });

  server.begin();
}

void http_server_handle() {
  server.handleClient();
}

static void handle_root() {
  byte mac_address[6];

  WiFi.macAddress(mac_address);

  if(server.hasArg("preset"))
    preset_set(server.arg("preset").c_str());

  if(server.hasArg("animation"))
    animation_set(server.arg("animation").c_str());
  if(server.hasArg("sequence"))
    animation_set(server.arg("animation").c_str());

  if(server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    uint8_t red, green, blue;

    red = strtol(server.arg("r").c_str(), 0, 16);
    green = strtol(server.arg("g").c_str(), 0, 16);
    blue = strtol(server.arg("b").c_str(), 0, 16);

    preset_rgb(red, green, blue);
  }

  if(server.hasArg("brightness"))
    leds_brightness(server.arg("brightness").toInt());

  if(server.hasArg("maximum_brightness"))
    leds_maximum_brightness(server.arg("maximum_brightness").toInt());

  if(server.hasArg("speed")) {
    float speed = server.arg("speed").toFloat();

    if(speed)
      animation_speed(speed);
  }

  String page = "<html><head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width'>"
    "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css' integrity='sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm' crossorigin='anonymous'>"
    "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/bootstrap-colorpicker/2.5.3/css/bootstrap-colorpicker.min.css' integrity='sha384-rgZjFxA67xGusHTOolh49vhPmf5NcM8W9OhdUvP7xZt65SQgCc/PI0iSQ4EY6XUA' crossorigin='anonymous'>"
    "<title>LED Control</title>"
    "</head><body>"
    "<div class='container' style='background-color: white'>"
    "<h1><a href='/'>LED Controls</a></h1>"
    "<div class='row'>"
    "<div class='card col-sm border-dark border-rounded'>"
    "<div class='card-body'>"
    "<h2 class='card-title'>Color Presets</h2>"
    "<form action='/'>"
    "<select name='preset' class='form-control'>";

  if(current_preset == NULL)
    page += "  <option value='' selected disabled hidden>none selected</option>";

  for(int i = 0; i < presets_length; i++) {
    page += "  <option value='";
    page += presets[i].name;
    if(&presets[i] == current_preset)
      page += "' selected>";
    else
      page += "'>";

    page += presets[i].name;
    page += "</option>";
  }

  page += "</select>"
    "<br/>"
    "</form>"
    "<hr/>"
    "<form action='/' class='form-inline'>"
    "<label for='r'>Red </label><input type='number' id='r' name='r' min='0' max='255' class='form-control'>"
    "<label for='g'>Green </label><input type='number' id='g' name='g' min='0' max='255' class='form-control'>"
    "<label for='b'>Blue </label><input type='number' id='b' name='b' min='0' max='255' class='form-control'>"
    "<br/><input type='submit' class='form-control btn btn-primary'>"
    "</form>"
    "<div style='float: clear;'></div>"
    "<hr/>"
    "<div id='color-picker-container'></div>"
    "</div>"
    "</div>"
    "<div class='card col-sm border-dark border-rounded'>"
    "<div class='card-body'>"
    "<h2 class='card-title'>Animations</h2>"
    "<form action='/'>"
    "<select id='animation' name='animation' class='form-control'>";

  if(current_animation == NULL)
    page += "  <option value='' selected disabled hidden>none selected</option>";

  bool any_animation_ignores_presets = false;
  for(int i = 0; i < animations_length; i++) {
    page += "  <option value='";
    page += animations[i].name;
    if(&animations[i] == current_animation)
      page += "' selected>";
    else
      page += "'>";

    page += animations[i].name;
    if(animations[i].ignores_presets) {
      any_animation_ignores_presets = true;
      page += " *";
    }
    page += "</option>";
  }

  page += "</select>";
  if(any_animation_ignores_presets)
    page += "<br/>* animation ignores color presets";

  page += "<br/>"
    "</form>"
    "<hr/>"
    "<form action='/' class='form'>"
    "<label for='speed'>Speed factor (0.5 = half, 2 = double)</label>"
    "<input type='number' step='0.1' class='form-control' id='speed' name='speed' size='4' value='" + String(animation_speed()) + "'>"
    "</form>"
    "<hr/>"
    "<a href='/stop' class='btn btn-info'>Stop Animation</a>"
    "</div>"
    "</div>"
    "</div><!- row ->"

    "<div class='row'>"
    "<div class='card col-sm border-dark border-rounded'>"
    "<div class='card-body'>"
    "<h2 class='card-title'>Brightness</h2>"
    "<div><form action='/'>"
    "  <label for='brightness'>Brightness (0-100%)";

  if(leds_maximum_brightness() != 100)
    page += " (maximum brightness " + String(leds_maximum_brightness()) + "%)";

  page += "</label>"
    "  <input type='number' step='0.1' class='form-control' id='brightness' name='brightness' min='0' max='100' value='" + String(leds_brightness()) + "'>"
    "</form></div>"

    "<a href='/off' class='btn btn-dark'>Turn Off</a><br/>"

    "</div>"
    "</div>"
    "</div><!- row ->"

    "<div class='row'>"
    "<div class='card col-sm border-dark border-rounded'>"
    "<div class='card-body'>"
    "<h2 class='card-title'>System</h2>"
    "<a href='/persist' class='btn btn-success'>Start up with current settings</a><br/>"
    "<a href='/clear_persist' class='btn btn-danger'>Clear start up settings</a><br/>"
    "<a href='/restart' class='btn btn-dark'>Restart</a><br/>"
    "<b>Hostname:</b> " + String(wifi_hostname()) + 
    " <b>IP address:</b> " + String(WiFi.localIP()[0]) + "."  + String(WiFi.localIP()[1]) + "."  + String(WiFi.localIP()[2]) + "."  + String(WiFi.localIP()[3]) +
    " <b>MAC address:</b> " + String(mac_address[0], 16) + ":" + String(mac_address[1], 16) + ":" + String(mac_address[2], 16) + ":" + String(mac_address[3], 16) + ":" + String(mac_address[4], 16) + ":" + String(mac_address[5], 16) +
    "<br/><b>Wifi network:</b> " + WiFi.SSID() +
    " <b>RSSI</b>: " + String(WiFi.RSSI()) +
    " <b>subnet mask:</b>" + String(WiFi.subnetMask()[0]) + "."  + String(WiFi.subnetMask()[1]) + "."  + String(WiFi.subnetMask()[2]) + "."  + String(WiFi.subnetMask()[3]) +
    " <b>default router:</b>" + String(WiFi.gatewayIP()[0]) + "."  + String(WiFi.gatewayIP()[1]) + "."  + String(WiFi.gatewayIP()[2]) + "."  + String(WiFi.gatewayIP()[3]) +
    "<br/><b>Number of LEDs:</b> " + String(NUM_LEDS) +
#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)
    " <b>LED controller:</b> " + STRINGIZE(LED_TYPE) +
    " <b>R-G-B order:</b> " + STRINGIZE(LED_RGB) +
    " <b>Data pin:</b> " + String(LED_DATA_PIN) +
#ifdef LED_CLOCK_PIN
    " <b>Clock pin:</b> " + String(LED_CLOCK_PIN) +
#else
    " <b>Clock pin:</b> unused" +
#endif
#ifdef USE_MQTT
    "<br/><b>MQTT (" + (mqtt_is_connected() ? "" : "not ") + "connected)broker:</b> " + MQTT_HOST + " <b>port:</b> " + String(MQTT_PORT) + " <b>username:</b> " + MQTT_USER + " <b>UUID:</b> " + MQTT_UUID +
#else
    "<b>No MQTT support</b>"
#endif
    "<br/><b>Files:</b>";

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
 
    while(file) {
      page += " " + String(file.name()) + " ";
      file.close();
      file = root.openNextFile();
    }

    root.close();

    page += "<br/><b>SPIFFS:</b> " + String(SPIFFS.usedBytes()) + " bytes used of " + String(SPIFFS.totalBytes()) + " available";
    page += "</div>"
    "</div>"
    "</div><!- row ->"

#ifdef HAS_BME280
    "<div class='row'>"
    "<div class='card col-sm border-dark border-rounded'>"
    "<div class='card-body'>"
    "<h2 class='card-title'>Environment</h2>"
    "<b>Temperature:</b> " + String(bme280_current_temperature()) + "°C"
    "<br/><b>Pressure:</b> " + String(bme280_current_pressure()) + ""
    "<br/><b>Humidity:</b> " + String(bme280_current_humidity()) + "%"
    "</div>"
    "</div>"
    "</div><!- row ->"
#endif

    "<div class='row'>"
    "Want to hack this?&nbsp; <a href='https://github.com/romkey/vendo'>https://github.com/romkey/vendo</a>"
    "</div>"
    "</div>"
    "<script src='https://code.jquery.com/jquery-3.2.1.min.js' integrity='sha384-xBuQ/xzmlsLoJpyjoggmTEz8OWUFM0/RC5BsqQBDX2v5cMvDHcMakNTNrHIW2I5f' crossorigin='anonymous'></script>"
    "<script src='https://cdnjs.cloudflare.com/ajax/libs/jquery-timeago/1.6.7/jquery.timeago.min.js' integrity='sha384-bIExnxLHdrb4/5cUciUyC490hqkTPFw6V1eUbG0gpKQ67B3dsT6KOdXvl5RycCM6' crossorigin='anonymous'></script>"
    "<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js' integrity='sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q' crossorigin='anonymous'></script>"
    "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js' integrity='sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl' crossorigin='anonymous'></script>"
    "<script src='https://cdn.jsdelivr.net/npm/@jaames/iro/dist/iro.min.js'></script>"
    "<script>"
    "$(function () {"
    "    var colorPicker = new iro.ColorPicker('#color-picker-container', { width: 150 });"
    "    function onColorChange(color, changes) {"
    "        $('body').css('background-color', color.hexString);"
    "        var rgb = color.rgb;"
    "        $.get('/?r=' + rgb.r + '&g=' + rgb.g + '&b=' + rgb.b);"
    "        $('#r').val(rgb.r); $('#g').val(rgb.g); $('#b').val(rgb.b);"
    "        console.log(color.hexString);"
    "    }"
    "    colorPicker.on('color:change', onColorChange);"
    "    $('select').change(function() {"
    "        this.form.submit();"
    "    });"
    "});"
    "</script>"
    "</body>"
    "</html>";

  server.send(200, "text/html", page);
}

static void handle_off() {
  preset_set("off");
  leds_off();
  current_preset = NULL;
  current_animation = NULL;
  animation_speed(1);
  server.sendHeader("Location", "/");
  server.send(302);
}

static void handle_start() {
  animation_start();
  server.send(302, "text/plain", "/");
}

static void handle_stop() {
  animation_stop();
  server.sendHeader("Location", "/");
  server.send(302);
}

static void handle_persist() {
  leds_persist();
  preset_persist();
  animation_persist();

  server.sendHeader("Location", "/");
  server.send(302);
}

static void handle_clear_persist() {
  leds_clear_persist();
  preset_clear_persist();
  animation_clear_persist();

  server.sendHeader("Location", "/");
  server.send(302);
}

static void handle_status() {
}
