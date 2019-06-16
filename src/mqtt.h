#pragma once

#include <Arduino.h>

#include <PubSubClient.h>

bool mqtt_connect(PubSubClient*);
void mqtt_handle();
void mqtt_callback(const char*, const byte*, unsigned);
