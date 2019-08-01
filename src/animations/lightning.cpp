#include "config.h"
#include "leds.h"
#include "animations.h"

#define FREQUENCY 50                                       // controls the interval between strikes
#define FLASHES   8                                          //the upper limit of flashes per strike
static unsigned int dimmer = 1;

static uint8_t ledstart;                                             // Starting location of a flash
static uint8_t ledlen;                                               // Length of a flash

static enum { FLASH_DELAY, INTERFLASH_DELAY } state;

unsigned animation_lightning(bool init) {
  static int flashCounter = 0;

  ledstart = random8(NUM_LEDS);                               // Determine starting location of flash
  ledlen = random8(NUM_LEDS-ledstart);                        // Determine length of flash (not to go beyond NUM_LEDS-1)
  
  for(; flashCounter < random8(3, FLASHES); flashCounter++) {
    if(state == INTERFLASH_DELAY) {
      if(flashCounter == 0)
	dimmer = 5;                         // the brightness of the leader is scaled down by a factor of 5
      else
	dimmer = random8(1, 3);                               // return strokes are brighter than the leader
    
      fill_solid(leds+ledstart,ledlen,CHSV(255, 0, 255/dimmer));

      FastLED.show();                       // Show a section of LED's
      state = FLASH_DELAY;
      return random8(4,10);                 // each flash only lasts 4-10 milliseconds
    } else {
      fill_solid(leds+ledstart,ledlen,CHSV(255,0,0));           // Clear the section of LED's
      FastLED.show();

      state = INTERFLASH_DELAY;
      if(flashCounter == 0)
	return 150;
      else
	return 50+random8(100);                                   // shorter delay between strokes
    }
  }
  
  flashCounter = 0;
  return random8(FREQUENCY)*100;                              // delay between strikes
}
