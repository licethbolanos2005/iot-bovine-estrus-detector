#ifndef GESTOR_OTA_H
#define GESTOR_OTA_H

#include <Arduino.h>

bool gestor_ota_init();
void gestor_ota_handle();
bool gestor_ota_wifi_conectado();

#endif