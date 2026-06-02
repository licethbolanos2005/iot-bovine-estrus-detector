#ifndef TIPOS_H
#define TIPOS_H

#include <Arduino.h>

// ─── Nivel de celo ────────────────────────────────────
typedef enum {
    CELO_NO_DETECTADO = 0,
    CELO_POSIBLE      = 1,
    CELO_PROBABLE     = 2,
    CELO_CONFIRMADO   = 3
} NivelCelo;

// ─── Datos completos del nodo ─────────────────────────
struct DatosNodo {
    // Timestamp
    uint16_t anio;
    uint8_t  mes, dia;
    uint8_t  hora, minuto, segundo;
    uint8_t rpm;

    // DS18B20
    float temperatura;

    // MPU6050
    float accel_x, accel_y, accel_z;
    float giro_x,  giro_y,  giro_z;
    float magnitud_accel;

    // INA219
    float voltaje_V;
    float corriente_mA;
    float potencia_mW;

    // FSR
    int   fsr_raw;
    float fsr_voltaje;

    // Detección celo
    NivelCelo nivel_celo;
    bool      temp_alta;
    bool      monta;
    uint8_t   ciclos_actividad;

    // Metadata
    char device_id[20];
    bool datos_validos;
};

// ─── Alerta de celo ───────────────────────────────────
struct AlertaCelo {
    NivelCelo nivel;
    char      mensaje[50];
    uint32_t  timestamp_ms;
};

#endif // TIPOS_H