#include <multiball/app.h>

#include "config.h"
#include "leds.h"
#include "animations.h"

#include <SPIFFS.h>
#include <FS.h>

/*
 * ANIMATIONS
 *
 * Animations are subroutines which take one argument, indicating whether they should
 * initialize themselves. In order to keep this file from becoming unreadable, each
 * animation routine is in its own file in the animations/ directory. Each animation
 * routine should keep its own variables static in order to avoid polluting the global
 * namespace and accidentally colliding with names used by other routines. Only the
 * routine name, which should start with animation_,  should be global. Add the routine
 * name to the animations[] array below to make it available to the user interface.
 *
 * Init will be called when the animation is first selected. On init the animation
 * routine should set up any state it needs in order to perform the animation (like
 * record the state of all the LEDs or set any state variables) and return.
 * 
 * When init is false, the animation routine should perform one frame of animation,
 * do any housekeeping it needs to, and return the number of milliseconds until the next
 * frame of animation.
 *
 * Animations should call FastLED.show() when they're ready to. The animation loop doesn't
 * do this for them.
 *
 * Animations should avoid calling delay() when possible and instead maintain state using
 * state variables and return the number of milliseconds till they need to resume. This
 * allows the code to also perform operations involving the network stack: service
 * commands via the web interface and MQTT and perform over-the-air-updates. Also do not use
 * FastLED's EVERY_N_MILLISECONDS() or EVERY_N_SECONDS() macros.
 *
 * Network activity will almost certainly disrupt the timing of an animation, but we
 * expect network activity to normally only happen when the user initiates a command, which
 * would change the presets or animation anyway.
 *
 * One exception is temperature updates - the code optionally supports periodically posting
 * the value of a temperature sensor to MQTT. If this is too disruptive of animations then
 * change the posting frequency to be less often.
 *
 * Animations have a speed factor - a floating point number which the frame delay is
 * divided by. Normally this is 1.0. To double the animation's speed, the speed factor
 * would be 2. To halve it, the speed factor would be 0.5. 
 *
 * Animation routines are called from a loop in leds_handle().
 *
 * They're kept in the animations array along with human-friendly names for them.
 */


void animation_setup() {
  animation_restore();
}

unsigned animation_alternating_blink(bool),
  animation_blink(bool),
  animation_fire(bool),
  animation_invert(bool),
  animation_lightning(bool),
  animation_march(bool),
  animation_throb(bool);

animation_t animations[] = {
  "alternating blink", animation_alternating_blink, false,
  "blink", animation_blink, false,
  "fire", animation_fire, true,
  "invert", animation_invert, false,
  "lightning", animation_lightning, true,
  "march", animation_march, false,
  "throb", animation_throb, false
};

unsigned animations_length = sizeof(animations)/sizeof(animation_t);

struct animation* current_animation  = NULL;

static float speed = 1;
static bool running = true;

void animation_start() {
  leds_on();

  App.updates_available(true);

  (*current_animation->animation)(true);
  running = true;
}

void animation_stop() {
  running = false;
}

void animation_speed(float desired_speed) {
  App.updates_available(true);

  speed = desired_speed;
}

float animation_speed(void) {
  return speed;
}

unsigned animate() {
  if(running)
    return (unsigned)((*current_animation->animation)(false) / speed);
  else
    return 10;
}

animation_t* animation_lookup(const char* name) {
  for(int i = 0; i < sizeof(animations)/sizeof(animation); i++)
    if(strcmp(animations[i].name, name) == 0)
      return &animations[i];

  return NULL;
}

void animation_set(animation_t* animation) {
  App.updates_available(true);

  leds_on();

  current_animation = animation;
  animation_start();
}

bool animation_set(const char* name) {
  animation_t* animation = animation_lookup(name);

  if(animation) {
    animation_set(animation);
    return true;
  }

  return false;
}

void animation_persist() {
  if(current_animation)
    App.config.set("animation-state", current_animation->name);
  else
    App.config.set("animation-state", "");

  App.config.set("animation-speed", String(speed));
}

void animation_clear_persist() {
  App.config.clear("animation-state");
  App.config.clear("animation-speed");
}

void animation_restore() {
  boolean success = false;
  String results;

  results = App.config.get("animation-state", &success);
  if(success)
    animation_set(results.c_str());

  results = App.config.get("animation-speed", &success);
  if(success)
    animation_speed(results.toFloat());
}
