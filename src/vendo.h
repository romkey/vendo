#pragma once

#define DDC_LED_UPDATE     "org.homebus.experimental.led-update"
#define DDC_LED_CONFIG     "org.homebus.experimental.led-config"
#define DDC_SYSTEM         "org.homebus.experimental.system"
#define DDC_DIAGNOSTIC     "org.homebus.experimental.diagnostic"
#define DDC_AIR_SENSOR     "org.homebus.experimental.air-sensor"
#define DDC_LED_PRESETS    "org.homebus.experimental.led-presets"
#define DDC_LED_ANIMATIONS "org.homebus.experimental.led-animations"

void vendo_setup();
void vendo_handle();

boolean vendo_led_status(char *buf, size_t buf_len);

  
