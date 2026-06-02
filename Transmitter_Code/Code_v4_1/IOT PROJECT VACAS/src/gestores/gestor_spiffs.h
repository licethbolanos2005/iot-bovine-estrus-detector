#ifndef GESTOR_SPIFFS_H
#define GESTOR_SPIFFS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#define CONFIG_PATH "/config.json"

bool   spiffs_init();
String spiffs_leer_config();
bool   spiffs_guardar_config(const String& json);
bool   spiffs_validar_json(const String& json);
String spiffs_config_default();

#endif