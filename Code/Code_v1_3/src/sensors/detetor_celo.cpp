#include "detector_celo.h"
#include <math.h>

// Contador persistente entre llamadas
static uint8_t ciclos_actividad_alta = 0;

void detector_celo_init() {
    ciclos_actividad_alta = 0;
}

NivelCelo detector_celo_analizar(
    const DatosMPU6050 &mpu,
    float temperatura,
    float voltaje_fsr,
    ResultadoCelo &resultado)
{
    // ── 1. Calcular magnitud del vector aceleración ──
    float magnitud = sqrt(
        mpu.accel_x * mpu.accel_x +
        mpu.accel_y * mpu.accel_y +
        mpu.accel_z * mpu.accel_z
    );

    // ── 2. Evaluar cada indicador ────────────────────
    bool actividad_alta      = (magnitud >= UMBRAL_ACTIVIDAD_ALTA);
    bool actividad_media     = (magnitud >= UMBRAL_ACTIVIDAD_MEDIA);
    bool temperatura_elevada = (temperatura >= UMBRAL_TEMP_ELEVADA);
    bool evento_monta        = (voltaje_fsr >= UMBRAL_FSR_MONTA);

    // ── 3. Actualizar contador de actividad ──────────
    if (actividad_alta) {
        if (ciclos_actividad_alta < 255) ciclos_actividad_alta++;
    } else {
        // Decremento gradual — no resetea inmediatamente
        if (ciclos_actividad_alta > 0) ciclos_actividad_alta--;
    }

    // ── 4. Determinar nivel de celo ──────────────────
    NivelCelo nivel = CELO_NO_DETECTADO;

    // Confirmado: monta + (actividad alta o temp elevada)
    if (evento_monta && (actividad_media || temperatura_elevada)) {
        nivel = CELO_CONFIRMADO;
    }
    // Probable: actividad sostenida + temperatura elevada
    else if (ciclos_actividad_alta >= CICLOS_ACTIVIDAD_ALERTA
             && temperatura_elevada) {
        nivel = CELO_PROBABLE;
    }
    // Posible: actividad alta sostenida sola
    else if (ciclos_actividad_alta >= CICLOS_ACTIVIDAD_ALERTA) {
        nivel = CELO_POSIBLE;
    }
    // Posible: solo temperatura elevada
    else if (temperatura_elevada) {
        nivel = CELO_POSIBLE;
    }
    // Posible: solo evento de monta aislado
    else if (evento_monta) {
        nivel = CELO_POSIBLE;
    }

    // ── 5. Llenar resultado ──────────────────────────
    resultado.nivel                 = nivel;
    resultado.actividad_alta        = actividad_alta;
    resultado.temperatura_elevada   = temperatura_elevada;
    resultado.evento_monta          = evento_monta;
    resultado.magnitud_accel        = magnitud;
    resultado.ciclos_actividad_alta = ciclos_actividad_alta;

    return nivel;
}

const char* detector_celo_nivel_str(NivelCelo nivel) {
    switch (nivel) {
        case CELO_NO_DETECTADO: return "No detectado";
        case CELO_POSIBLE:      return "POSIBLE";
        case CELO_PROBABLE:     return "PROBABLE";
        case CELO_CONFIRMADO:   return "*** CONFIRMADO ***";
        default:                return "Desconocido";
    }
}