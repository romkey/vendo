#include "config.h"

#include "http_server.h"

#include "leds.h"
#include "presets.h"
#include "animations.h"

#include "multiball/bme280.h"

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
//  /?speed=XXX
// /on
// /off
// /start
// /stop
// /status.json

static void handle_root(), handle_on(), handle_off(), handle_start(), handle_stop(), handle_status();

void http_server_setup() {
  server.on("/", handle_root);
  server.on("/on", handle_on);
  server.on("/off", handle_off);
  server.on("/stop", handle_stop);
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
    leds_brightness(atoi(server.arg("brightness").c_str()));
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
    "<form action='/'>"
    "<label for='preset'>Light pattern presets</label>"
    "<select name='preset' class='form-control'>";

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
    "<form action='/'>"
    "<label for='animation'>Animations</label>"
    "<select id='animation' name='animation' class='form-control'>";

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
    "<a href='/stop' class='btn btn-info'>Stop Animation</a>"
    "<div><form action='/'>"
    "  <label for='brightness'>Brightness (0-100%)</label>"
    "  <input type='number' id='brightness' name='brightness' min='0' max='100' value='" + String(leds_brightness()) + "'>"
    "</form></div>"
    "<form action='/' class='form'>"
    "<label for='brightness'>Animation speed factor (0.5 = half speed, 2 = double speed)</label>"
    "<input type='text' id='speed' name='speed' size='4' value='" + String(animation_speed()) + "'>"
    "</form>"
    "<form action='/' class='form-inline'>"
    "<div id='color-picker-container'></div>"
    "<label for='r'>Red </label><input type='number' id='r' name='r' min='0' max='255' class='form-control'>"
    "<label for='g'>Green </label><input type='number' id='g' name='g' min='0' max='255' class='form-control'>"
    "<label for='b'>Blue </label><input type='number' id='b' name='b' min='0' max='255' class='form-control'>"
    "<br/><input type='submit' class='form-control btn btn-primary'>"
    "</form>"
#ifdef HAS_BME280
    "<div>"
    "Current temperature is ";
  page += String(bme280_current_temperature());
  page += "°C</div>"
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

static void handle_on() {
  leds_on();
  server.send(302, "text/plain", "/");
}

static void handle_off() {
  preset_set("off");
  animation_stop();
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

static void handle_status() {
}
