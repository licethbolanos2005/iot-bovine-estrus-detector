#ifndef DETECTOR_CELO_H
#define DETECTOR_CELO_H

#include "../tipos.h"  // NivelCelo ya está definido aquí
#include "sensor_mpu6050.h"

struct ResultadoCelo {
    NivelCelo nivel;
    bool      actividad_alta;
    bool      temperatura_elevada;
    bool      evento_monta;
    float     magnitud_accel;
    uint8_t   ciclos_actividad_alta;
};

void        detector_celo_init();
NivelCelo   detector_celo_analizar(
                const DatosMPU6050 &mpu,
                float temperatura,
                float voltaje_fsr,
                ResultadoCelo &resultado);
const char* detector_celo_nivel_str(NivelCelo nivel);

#endif