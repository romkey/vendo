# ^H Vending Machine LED Lighting Controller

This repo has the hardware plans and code for the lighting inside the vending machine at [Control H](https://pdxhackerspace.org).

## Hardware

The hardware is based on an ESP32 which uses a level shifter to drive an LED strip. We used an LPD8806-based strip, but the code can work with any strip that the FastLED library supports. Some LED strips won't even need a level shifter because the LED controllers will handle 3.3V signals.

## Software

The software is written using the Arduino Core for the ESP32. It's structured to build with [PlatformIO](https://platformio.org/), not the Arduino IDE.

## Control Interface

The controller has two control interfaces, implemented over HTTP and MQTT.

Users can:
- choose preset light patterns
- choose animations
- adjust the animation speed
- adjust the brightness


## Extras

The controller also uses a BME280 to monitor the temperature inside the vending machine.
