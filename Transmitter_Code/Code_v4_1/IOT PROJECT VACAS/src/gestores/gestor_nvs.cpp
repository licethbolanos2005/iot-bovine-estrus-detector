#include "gestor_nvs.h"
#include "../config.h"
#include <Preferences.h>   // Librería NVS de Arduino-ESP32 (incluida en el framework)

// Namespace NVS — máx 15 caracteres
static const char* NVS_NS = "bovino_cfg";

static Preferences prefs;

bool gestor_nvs_init() {
    // true = solo lectura; false = lectura/escritura
    bool ok = prefs.begin(NVS_NS, false);
    if (!ok) {
        Serial.println("[NVS] Error abriendo namespace");
    } else {
        Serial.println("[NVS] OK");
    }
    return ok;
}

// ── Device ID ─────────────────────────────────────────
void gestor_nvs_leer_device_id(char *buf, size_t len) {
    String id = prefs.getString("device_id", DEVICE_ID);  // default de config.h
    strncpy(buf, id.c_str(), len - 1);
    buf[len - 1] = '\0';
}

void gestor_nvs_guardar_device_id(const char *id) {
    prefs.putString("device_id", id);
}

// ── Umbrales ──────────────────────────────────────────
float gestor_nvs_leer_umbral_actividad_alta() {
    return prefs.getFloat("act_alta", UMBRAL_ACTIVIDAD_ALTA);
}

float gestor_nvs_leer_umbral_temp_elevada() {
    return prefs.getFloat("temp_elev", UMBRAL_TEMP_ELEVADA);
}

float gestor_nvs_leer_umbral_fsr_monta() {
    return prefs.getFloat("fsr_monta", UMBRAL_FSR_MONTA);
}

uint8_t gestor_nvs_leer_ciclos_alerta() {
    return (uint8_t)prefs.getUInt("ciclos_al", CICLOS_ACTIVIDAD_ALERTA);
}

void gestor_nvs_guardar_umbral_actividad_alta(float val) {
    prefs.putFloat("act_alta", val);
}

void gestor_nvs_guardar_umbral_temp_elevada(float val) {
    prefs.putFloat("temp_elev", val);
}

void gestor_nvs_guardar_umbral_fsr_monta(float val) {
    prefs.putFloat("fsr_monta", val);
}

void gestor_nvs_guardar_ciclos_alerta(uint8_t val) {
    prefs.putUInt("ciclos_al", val);
}

// ── Parámetros LoRa ───────────────────────────────────
float gestor_nvs_leer_lora_freq() {
    return prefs.getFloat("lora_freq", LORA_FREQ);
}

uint8_t gestor_nvs_leer_lora_sf() {
    return (uint8_t)prefs.getUInt("lora_sf", LORA_SF);
}

void gestor_nvs_guardar_lora_freq(float freq) {
    prefs.putFloat("lora_freq", freq);
}

void gestor_nvs_guardar_lora_sf(uint8_t sf) {
    prefs.putUInt("lora_sf", sf);
}

// ── Factory reset ─────────────────────────────────────
void gestor_nvs_borrar_todo() {
    prefs.clear();
    Serial.println("[NVS] Configuracion borrada — usando defaults");
}