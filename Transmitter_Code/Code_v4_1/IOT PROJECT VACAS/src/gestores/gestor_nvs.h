#ifndef GESTOR_NVS_H
#define GESTOR_NVS_H

#include <Arduino.h>

// ── Inicializar NVS ────────────────────────────────────
bool gestor_nvs_init();

// ── Device ID ─────────────────────────────────────────
void gestor_nvs_leer_device_id(char *buf, size_t len);
void gestor_nvs_guardar_device_id(const char *id);

// ── Umbrales de detección (con defaults de config.h) ──
float gestor_nvs_leer_umbral_actividad_alta();
float gestor_nvs_leer_umbral_temp_elevada();
float gestor_nvs_leer_umbral_fsr_monta();
uint8_t gestor_nvs_leer_ciclos_alerta();

void gestor_nvs_guardar_umbral_actividad_alta(float val);
void gestor_nvs_guardar_umbral_temp_elevada(float val);
void gestor_nvs_guardar_umbral_fsr_monta(float val);
void gestor_nvs_guardar_ciclos_alerta(uint8_t val);

// ── Parámetros LoRa ───────────────────────────────────
float   gestor_nvs_leer_lora_freq();
uint8_t gestor_nvs_leer_lora_sf();
void    gestor_nvs_guardar_lora_freq(float freq);
void    gestor_nvs_guardar_lora_sf(uint8_t sf);

// ── Borrar todo (factory reset) ───────────────────────
void gestor_nvs_borrar_todo();

#endif // GESTOR_NVS_H