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
  server.on("/start", handle_on);
  server.on("/stop", handle_off);
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

}

static void handle_on() {
  leds_on();
}

static void handle_off() {
  leds_off();
}

static void handle_start() {
}


static void handle_status() {
}
