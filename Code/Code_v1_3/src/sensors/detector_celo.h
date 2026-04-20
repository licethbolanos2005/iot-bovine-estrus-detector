#ifndef DETECTOR_CELO_H
#define DETECTOR_CELO_H

#include "sensor_mpu6050.h"
#include "sensor_ds18b20.h"
#include "sensor_fsr.h"

// ─── Umbrales de detección ────────────────────────────
// Actividad: magnitud del vector aceleración por encima
// de la gravedad normal (9.81 m/s²)
#define UMBRAL_ACTIVIDAD_ALTA     12.0f   // m/s²
#define UMBRAL_ACTIVIDAD_MEDIA     10.5f   // m/s²

// Temperatura corporal bovina normal: 38.0 - 39.5 °C
// Durante el celo puede subir hasta 0.5 °C
#define UMBRAL_TEMP_ELEVADA        39.5f   // °C
#define UMBRAL_TEMP_NORMAL_MAX     39.0f   // °C

// FSR: voltaje mínimo para considerar evento de monta
#define UMBRAL_FSR_MONTA           2.0f    // V

// Tiempo mínimo de actividad alta para alertar (en ciclos)
#define CICLOS_ACTIVIDAD_ALERTA    3

// ─── Niveles de alerta ────────────────────────────────
typedef enum {
    CELO_NO_DETECTADO   = 0,
    CELO_POSIBLE        = 1,
    CELO_PROBABLE       = 2,
    CELO_CONFIRMADO     = 3
} NivelCelo;

// ─── Resultado del análisis ───────────────────────────
struct ResultadoCelo {
    NivelCelo nivel;
    bool      actividad_alta;
    bool      temperatura_elevada;
    bool      evento_monta;
    float     magnitud_accel;
    uint8_t   ciclos_actividad_alta;
};

// ─── Funciones públicas ───────────────────────────────
void        detector_celo_init();
NivelCelo   detector_celo_analizar(
                const DatosMPU6050 &mpu,
                float temperatura,
                float voltaje_fsr,
                ResultadoCelo &resultado);
const char* detector_celo_nivel_str(NivelCelo nivel);

#endif // DETECTOR_CELO_H