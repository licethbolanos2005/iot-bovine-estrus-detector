#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Inicia WiFi en modo AP+STA usando el JSON de configuración.
// Retorna true si conectó a la red local, false si solo quedó en modo AP.
bool startWifi(const String& configJson);

// Retorna la IP actual (STA si conectado, AP si no).
String getIP();

// Retorna true si está conectado a una red WiFi.
bool isConnected();