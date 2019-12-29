#! /bin/bash

~/.platformio/packages/tool-mkspiffs/mkspiffs_espressif32_arduino -c data -p 256 -b 4096 -s 1507328 .pio/build/lolin32/spiffs.bin
~/.platformio/packages/framework-arduinoespressif32/tools/espota.py -d -s -f .pio/build/lolin32/spiffs.bin -i $1 -p 3232 -r -d
