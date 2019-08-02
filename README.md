# ^H Vending Machine LED Lighting Controller

This repo has the hardware plans and code for the lighting inside the vending machine at [Control H](https://pdxhackerspace.org).

## Hardware

The hardware is based on an ESP32 which uses a level shifter to drive an LED strip. We used an LPD8806-based strip, but the code can work with any strip that the FastLED library supports. Some LED strips won't even need a level shifter because the LED controllers will handle 3.3V signals.

I chose the ESP32 because it's cheap (about $5-6 from AliExpress, or twice that on Amazon) and has a lot of head room. It has two CPU cores; one runs the network stack and the other runs the Arduino applicaion. It should be able to be responsive to the network while still running smooth animations. With some minor changes to the network code, vendo could also easily run on an ESP32 or on an Arduino.

i've designed a simple printed circuit board called a "discoball" which has all the hardware you'll need to do this. It's very simple. I'm still debugging it and will publish it in its own repository soon.

## Software

The software is written using the Arduino Core for the ESP32. It's structured to build with [PlatformIO](https://platformio.org/), not the Arduino IDE. PlatformIO changes the build process but shouldn't affect the actual code at all. I use PlatformIO because it's fast, much less cumbersome than the Arduino IDE, and has good library management. It's also (recently) fully open source.

If you want to build this project using the Arduino IDE, and rename `src/main.cpp` to `src/vendo.ino` and the `src` directory to `vendo` and manually install the libraries that are listed in`platformio.ini`.

## Control Interface

The controller has two control interfaces, implemented over HTTP and MQTT.

Users can:
- choose preset light patterns
- choose animations
- adjust the animation speed

To control over HTTP, find the IP address of the controller and load http://controller-ip-address in a web browser.

MQTT supports the following commands:
- restart
- off
- stop
- speed
- rgb RRGGBB
- preset PRESET_NAME
- animation ANIMATION_NAME

`RRGGBB` is the red-green-blue value in hexadecimal, no leading `0x`. for instance, `rgb FF0000` would be equivalent to `preset red`.

## Extras

The controller also uses a BME280 to monitor the temperature inside the vending machine.


## Implementation details

### Presets

 Presets simply set the LEDs to a selected color or pattern.

The preset routine should call FastLED.show() when it's done.

Setting a preset stops the current animation.

### Animations

Animations are subroutines which take one argument, indicating whether they should
initialize themselves. In order to keep `src/animations.cpp` from becoming unreadable, each
animation routine is in its own file in the `src/animations/` directory. Each animation
routine should keep its own variables static in order to avoid polluting the global
namespace and accidentally colliding with names used by other routines. Only the
routine name, which should start with animation_,  should be global. Add the routine
name to the animations[] array below to make it available to the user interface.

Init will be called when the animation is first selected. On init the animation
routine should set up any state it needs in order to perform the animation (like
record the state of all the LEDs or set any state variables) and return.

When init is false, the animation routine should perform one frame of animation,
do any housekeeping it needs to, and return the number of milliseconds until the next
frame of animation.

Animations should call `FastLED.show()` when they're ready to. The animation loop doesn't
do this for them.

Animations should avoid calling `delay()` when possible and instead should maintain state using
state variables and return the number of milliseconds till they need to resume. This
allows the code to also perform operations involving the network stack: service
commands via the web interface and MQTT and perform over-the-air-updates. Also do not use
FastLED's `EVERY_N_MILLISECONDS()` or `EVERY_N_SECONDS()` macros.

Network activity will almost certainly disrupt the timing of an animation, but we
expect network activity to normally only happen when the user initiates a command, which
would change the presets or animation anyway.

One exception is temperature updates - the code optionally supports periodically posting
the value of a temperature sensor to MQTT. If this is too disruptive of animations then
change the posting frequency to be less often.

Animations have a speed factor - a floating point number which the frame delay is
divided by. Normally this is 1.0. To double the animation's speed, the speed factor
would be 2. To halve it, the speed factor would be 0.5. 

Animation routines are called from a loop in `leds_handle()`.

They're kept in the animations array along with human-friendly names for them.

### Firmware updates

Vendo supports over-the-air (OTA) updates. Using PlatformIO you can build and install an update:

    platformio run -t upload --upload-port NAME-OR-IP-ADDRESS


## License

Code is licensed under the [MIT License](https://romkey.mit-license.org).
