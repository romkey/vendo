#include "http_server.h"

#include "leds.h"
#include "presets.h"
#include "animations.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

static WiFiClient client;
static WebServer server(80);

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

  if(server.hasArg("brightness"))
    leds_brightness(atoi(server.arg("brightness").c_str()));
  if(server.hasArg("speed"))
    animation_speed(atoi(server.arg("speed").c_str()));

  String page = "<html><head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width'>"
    "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css' integrity='sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm' crossorigin='anonymous'>"
    "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/bootstrap-colorpicker/2.5.3/css/bootstrap-colorpicker.min.css' integrity='sha384-rgZjFxA67xGusHTOolh49vhPmf5NcM8W9OhdUvP7xZt65SQgCc/PI0iSQ4EY6XUA' crossorigin='anonymous'>"
    "<title>LED Control</title>"
    "</head><body>"
    "<div class='container'>"
    "<h1><a href='/'>LED Controls</a></h1>"
    "<form action='/'>"
    "<label for='preset'>Light pattern presets</label>"
    "<select name='preset' class='form-control'>";

  for(int i = 0; i < presets_length; i++) {
    page += "  <option value='";
    page += presets[i].name;
    if(server.hasArg("preset") && server.arg("preset") == presets[i].name)
      page += "' selected>";
    else
      page += "'>";

    page += presets[i].name;
    page += "</option>";
  }

  page += "</select>"
    "<br/><input type='submit' class='form-control btn btn-primary'>"
    "</form>"
    "<form action='/'>"
    "<label for='animation'>Animations</label>"
    "<select id='animation' name='animation' class='form-control'>";

  for(int i = 0; i < animations_length; i++) {
    page += "  <option value='";
    page += animations[i].name;
    if(server.hasArg("animation") && server.arg("animation") == animations[i].name)
      page += "' selected>";
    else
      page += "'>";

    page += animations[i].name;
    page += "</option>";
  }

  page += "</select>"
    "<br/><input type='submit' class='form-control btn btn-primary'>"
    "</form>"
    "<a href='/stop' class='btn btn-info'>Stop Animation</a><a href='/start' class='btn btn-info'>Start Animation</a>"
    //    "<form action='/'>"
    //    "<label for='brightness'>Brightness</label>"
    //    "<input type='number' id='brightness' name='brightness' min='0' max='100'>"
    //    "</form>"
    "<form action='/'>"
    "<label for='brightness'>Animation speed</label>"
    "<input type='text' id='speed' name='speed' size='4' placeholder='% speed'>"
    "</form>"
    "<form action='/' class='form-inline'>"
    //    "<div id='pick-a-color' class='input-group' title='Using format option'>"
    //    "<input name='rgb' type='text' class='form-control' value='F0F0F0' />"
    //    "<span class='input-group-append'>"
    //    "<span class='input-group-text colorpicker-input-addon'><i></i></span>"
    //    "</span>"
    //    "</div>"
    "<label for='r'>Red </label><input type='number' id='r' name='r' min='0' max='255' class='form-control'>"
    "<label for='g'>Green </label><input type='number' id='g' name='g' min='0' max='255' class='form-control'>"
    "<label for='b'>Blue </label><input type='number' id='b' name='b' min='0' max='255' class='form-control'>"
    "<br/><input type='submit' class='form-control btn btn-primary'>"
    "</form>"
    "</div>"
    "<script src='https://code.jquery.com/jquery-3.2.1.slim.min.js' integrity='sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN' crossorigin='anonymous'></script>"
    "<script src='https://cdnjs.cloudflare.com/ajax/libs/jquery-timeago/1.6.7/jquery.timeago.min.js' integrity='sha384-bIExnxLHdrb4/5cUciUyC490hqkTPFw6V1eUbG0gpKQ67B3dsT6KOdXvl5RycCM6' crossorigin='anonymous'></script>"
    "<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js' integrity='sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q' crossorigin='anonymous'></script>"
    "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js' integrity='sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl' crossorigin='anonymous'></script>"
    "<script src='https://cdnjs.cloudflare.com/ajax/libs/bootstrap-colorpicker/2.5.3/js/bootstrap-colorpicker.min.js' integrity='sha384-EhfNLT6NEylLH3+7zgUOZrC8Qy8BzFCEYJPTeJ2kzre0URgJACC0gcIgxYGStEC6' crossorigin='anonymous'></script>"
    "<script>"
    "$(function () {"
    "  $('#pick-a-color').colorpicker({ useAlpha: false, horizontal: true, useHashPrefix: false });"
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
  server.send(302, "text/plain", "/");
}

static void handle_start() {
  animation_start();
  server.send(302, "text/plain", "/");
}

static void handle_stop() {
  animation_stop();
  server.send(302, "text/plain", "/");
}

static void handle_status() {
}
