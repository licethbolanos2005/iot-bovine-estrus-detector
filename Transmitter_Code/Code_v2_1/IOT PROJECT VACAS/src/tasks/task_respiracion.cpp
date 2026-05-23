#include "task_respiracion.h"
#include "queues_globales.h"
#include "../config.h"
#include "../sensors/sensor_mpu6050.h"
#include <math.h>

// ─── Parámetros del algoritmo ─────────────────────────
#define MUESTREO_HZ           20      // Hz de muestreo
#define MUESTREO_MS           50      // 1000/MUESTREO_HZ
#define ALPHA_FILTRO          0.15f   // Suavizado pasa-bajos
#define UMBRAL_PICO_MS2       0.8f    // Sensibilidad de pico
#define INTERVALO_MIN_MS      1500    // 40 RPM máx
#define INTERVALO_MAX_MS      8000    // 7.5 RPM mín
#define BUFFER_INTERVALOS     8       // Ventana para promediar

void task_respiracion(void *pvParameters) {
    Serial.println("[TASK_RESPIRACION] Iniciada");

    // Esperar a que los demás sensores estén listos
    vTaskDelay(pdMS_TO_TICKS(3000));

    // ── Variables de estado ──────────────────────────
    float accel_z_filt   = 0.0f;
    float accel_z_prev   = 0.0f;
    float pendiente_prev = 0.0f;

    uint32_t timestamp_pico_anterior = 0;
    uint32_t intervalos[BUFFER_INTERVALOS] = {0};
    uint8_t  idx_intervalo  = 0;
    uint8_t  cuenta_picos   = 0;

    // Inicialización del filtro
    if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(2000)) == pdTRUE) {
        DatosMPU6050 mpu;
        if (mpu6050_leer(mpu)) {
            accel_z_filt = mpu.accel_z;
            accel_z_prev = mpu.accel_z;
        }
        xSemaphoreGive(mutex_i2c);
    }

    for (;;) {
        // ── Leer MPU6050 protegido con mutex ─────────
        DatosMPU6050 mpu;
        bool lectura_ok = false;

        if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(100)) == pdTRUE) {
            lectura_ok = mpu6050_leer(mpu);
            xSemaphoreGive(mutex_i2c);
        }

        if (lectura_ok) {
            // ── Filtro pasa-bajos exponencial ────────
            accel_z_filt = ALPHA_FILTRO * mpu.accel_z +
                           (1.0f - ALPHA_FILTRO) * accel_z_filt;

            // ── Calcular pendiente ───────────────────
            float pendiente = accel_z_filt - accel_z_prev;

            // ── Detectar pico (cambio de + a -) ──────
            bool es_pico = (pendiente_prev > 0) &&
                           (pendiente < 0) &&
                           (fabs(accel_z_filt - accel_z_prev) > UMBRAL_PICO_MS2);

            if (es_pico) {
                uint32_t ahora = millis();

                if (timestamp_pico_anterior > 0) {
                    uint32_t delta = ahora - timestamp_pico_anterior;

                    // Validar intervalo razonable
                    if (delta >= INTERVALO_MIN_MS && delta <= INTERVALO_MAX_MS) {
                        intervalos[idx_intervalo] = delta;
                        idx_intervalo = (idx_intervalo + 1) % BUFFER_INTERVALOS;
                        if (cuenta_picos < BUFFER_INTERVALOS) cuenta_picos++;

                        // ── Calcular RPM promedio ────
                        uint32_t suma = 0;
                        for (uint8_t i = 0; i < cuenta_picos; i++) {
                            suma += intervalos[i];
                        }
                        float promedio_ms = (float)suma / cuenta_picos;
                        float rpm = 60000.0f / promedio_ms;

                        // ── Guardar en datos compartidos ─
                        if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(100)) == pdTRUE) {
                            datos_compartidos.rpm = (uint8_t)rpm;
                            xSemaphoreGive(mutex_datos);
                        }

                        Serial.printf("[RESPIRACION] Pico! RPM=%.1f (intervalo=%lums)\n",
                            rpm, delta);
                    }
                }

                timestamp_pico_anterior = ahora;
            }

            pendiente_prev = pendiente;
            accel_z_prev   = accel_z_filt;
        }

        vTaskDelay(pdMS_TO_TICKS(MUESTREO_MS));
    }
}