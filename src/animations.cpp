#include "config.h"
#include "leds.h"
#include "animations.h"

/*
 * ANIMATIONS
 *
 * Animations are subroutines statically scoped to this file which take one argument,
 * indicating whether they should initialize themselves.
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
 * commands via the web interface and MQTT and perform over-the-air-updates.
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


/****
 **** definitions for MasterTwinkle by https://gist.github.com/kriegsman/88954aae22b03a664081
 ****/
#define PEAK_COLOR       CRGB(64,0,64)

#define DELTA_COLOR_UP   CRGB(4,0,4)

// Amount to decrement the color by each loop as it gets dimmer:
#define DELTA_COLOR_DOWN CRGB(1,0,1)

// Chance of each pixel starting to brighten up.  
// 1 or 2 = a few brightening pixels at a time.
// 10 = lots of pixels brightening at a time.
#define CHANCE_OF_TWINKLE 1

enum { SteadyDim, GettingBrighter, GettingDimmerAgain };

static unsigned twinkle(bool init) {
  static uint8_t PixelState[NUM_LEDS];
  static CRGB original_leds[NUM_LEDS];

  if(init) {
    for(int i = 0; i < NUM_LEDS; i++) {
      PixelState[i] = SteadyDim;
      original_leds[i] = leds[i];
    }

    //    fill_solid( leds, NUM_LEDS, BASE_COLOR);

    return 0;
  }

  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( PixelState[i] == SteadyDim) {
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < CHANCE_OF_TWINKLE) {
        PixelState[i] = GettingBrighter;
      }
      
    } else if( PixelState[i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[i] >= PEAK_COLOR ) {
        PixelState[i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        leds[i] += DELTA_COLOR_UP;
      }
      
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[i] <= original_leds[i] ) {
        leds[i] = original_leds[i]; // reset to exact base color, in case we overshot
        PixelState[i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[i] -= DELTA_COLOR_DOWN;
      }
    }
  }
}


static unsigned march(bool init) {
  CRGB last = leds[0];

  if(init)
    return 0;

  for(int i = 0; i < NUM_LEDS - 1 ; i++)
    leds[i] = leds[i+1];

  leds[NUM_LEDS-1] = last;
  FastLED.show();

  return 1000;
}

static unsigned throb(bool init) {
  static uint8_t brightness = 255;
  static int8_t direction = -1;
  static CRGB old_leds[NUM_LEDS];

  if(init) {
    for(int i = 0; i < NUM_LEDS; i++)
      old_leds[i] = leds[i];

    return 0;
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = old_leds[i];
    leds[i].nscale8_video(brightness);
  }

  FastLED.show();

  if(brightness == 255)
    direction = -1;

  if(brightness == 0)
    direction = 1;

  brightness += direction;

  return 10;
}

static unsigned invert(bool init) {
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = -leds[i];

  FastLED.show();

  return 1000;
}

static unsigned blink(bool init) {
  static CRGB old_leds[NUM_LEDS];
  static bool onoff = false;

  if(onoff) {
    for(int i = 0; i < NUM_LEDS; i++)
      leds[i] = old_leds[i];

    onoff = false;
  } else {
    for(int i = 0; i < NUM_LEDS; i++) {
      old_leds[i] = leds[i];
      leds[i] = 0;
    }

    onoff = true;
  }

  FastLED.show();

  return 1000;
}

static unsigned alternating_blink(bool init) {
  static CRGB old_leds[NUM_LEDS];
  static uint8_t offset = 0;

  if(init) {
    for(int i = 0; i < NUM_LEDS; i++)
      old_leds[i] = leds[i];

    return 0;
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    CRGB temp = leds[i];

    leds[i] = old_leds[i];
    old_leds[i] = temp;
  }

  offset +=1;
  offset %= 2;

  FastLED.show();

  return 1000;
}

animation_t animations[] = {
  "blink", blink,
  "alternating blink", alternating_blink,
  "invert", invert,
  "march", march,
  "throb", throb,
  "twinkle", twinkle
};

unsigned animations_length = sizeof(animations)/sizeof(animation_t);

struct animation* current_animation  = NULL;

static float speed = 1;
static bool running = true;

void animation_start() {
  (*current_animation->animation)(true);
  running = true;
}

void animation_stop() {
  running = false;
}

void animation_speed(float desired_speed) {
  speed = desired_speed;
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
