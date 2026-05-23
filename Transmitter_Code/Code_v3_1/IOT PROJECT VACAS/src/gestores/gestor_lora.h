#ifndef GESTOR_LORA_H
#define GESTOR_LORA_H

#include <Arduino.h>

bool gestor_lora_init();
bool gestor_lora_transmitir(const char *payload);

#endif