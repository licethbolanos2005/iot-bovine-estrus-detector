#ifndef GESTOR_WIFI_H
#define GESTOR_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

bool   wifi_iniciar(const String& configJson);
String wifi_ip();
bool   wifi_conectado();
bool   wifi_iniciar_mdns(const char* hostname);   // ← NUEVO

#endif