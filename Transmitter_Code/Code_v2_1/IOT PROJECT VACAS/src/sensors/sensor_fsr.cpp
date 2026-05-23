#include "sensor_fsr.h"
#include "../config.h"
#include <Arduino.h>

#define ADC_RESOLUCION  4095.0f
#define VOLTAJE_REF     3.3f

void fsr_init() {
    pinMode(PIN_FSR_ADC, INPUT);
    // ESP32-S3 requiere configurar atenuación ADC
    analogSetAttenuation(ADC_11db);
}

int fsr_leer_raw() {
    return analogRead(PIN_FSR_ADC);
}

float fsr_leer_voltaje() {
    return (fsr_leer_raw() / ADC_RESOLUCION) * VOLTAJE_REF;
}