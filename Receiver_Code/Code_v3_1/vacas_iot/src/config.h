#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#define CONFIG_PATH "/config.json"

// Inicializa SPIFFS. Retorna true si ok.
bool initSPIFFS();

// Lee config.json a un String. Retorna "" si falla.
String readConfig();

// Guarda un String JSON en config.json. Retorna true si ok.
bool saveConfig(const String& json);

// Valida que un String sea JSON válido. Retorna true si ok.
bool validateJson(const String& json);

// Retorna el JSON de configuración por defecto.
String getDefaultConfig();