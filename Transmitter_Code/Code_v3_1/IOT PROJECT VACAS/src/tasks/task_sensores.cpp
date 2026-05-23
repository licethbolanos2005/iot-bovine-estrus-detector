#include "task_sensores.h"
#include "queues_globales.h"
#include "../config.h"
#include "../sensors/sensor_ds18b20.h"
#include "../sensors/sensor_mpu6050.h"
#include "../sensors/sensor_ina219.h"
#include "../sensors/sensor_ds3232.h"
#include "../sensors/sensor_fsr.h"
#include <math.h>

QueueHandle_t     queue_datos       = NULL;
QueueHandle_t     queue_alertas     = NULL;
SemaphoreHandle_t mutex_i2c         = NULL;
SemaphoreHandle_t mutex_datos       = NULL;
DatosNodo         datos_compartidos = {};

void task_sensores(void *pvParameters) {
    Serial.println("[TASK_SENSORES] Iniciada");

    DatosNodo datos;
    strncpy(datos.device_id, DEVICE_ID, sizeof(datos.device_id));

    datos.anio             = 0;
    datos.mes              = 0;
    datos.dia              = 0;
    datos.hora             = 0;
    datos.minuto           = 0;
    datos.segundo          = 0;
    datos.temperatura      = 0.0f;
    datos.accel_x          = 0.0f;
    datos.accel_y          = 0.0f;
    datos.accel_z          = 0.0f;
    datos.giro_x           = 0.0f;
    datos.giro_y           = 0.0f;
    datos.giro_z           = 0.0f;
    datos.magnitud_accel   = 0.0f;
    datos.voltaje_V        = 0.0f;
    datos.corriente_mA     = 0.0f;
    datos.potencia_mW      = 0.0f;
    datos.fsr_raw          = 0;
    datos.fsr_voltaje      = 0.0f;
    datos.nivel_celo       = CELO_NO_DETECTADO;
    datos.temp_alta        = false;
    datos.monta            = false;
    datos.ciclos_actividad = 0;
    datos.datos_validos    = false;
    datos.rpm = 0;

    for (;;) {
        datos.datos_validos = true;

        // ── DS18B20 — 1-Wire, sin mutex ──────────────
        float temp = ds18b20_leer_temperatura();
        datos.temperatura = (temp != DS18B20_DESCONECTADO) ? temp : 0.0f;

        // ── FSR — ADC, sin mutex ─────────────────────
        datos.fsr_raw     = fsr_leer_raw();
        datos.fsr_voltaje = fsr_leer_voltaje();

        // ── Bus I2C — protegido con mutex ────────────
        if (xSemaphoreTake(mutex_i2c, pdMS_TO_TICKS(1000)) == pdTRUE) {

            DatosRTC rtc;
            if (ds3232_leer(rtc)) {
                datos.anio    = rtc.anio;
                datos.mes     = rtc.mes;
                datos.dia     = rtc.dia;
                datos.hora    = rtc.hora;
                datos.minuto  = rtc.minuto;
                datos.segundo = rtc.segundo;
            }

            DatosMPU6050 mpu;
            if (mpu6050_leer(mpu)) {
                datos.accel_x = mpu.accel_x;
                datos.accel_y = mpu.accel_y;
                datos.accel_z = mpu.accel_z;
                datos.giro_x  = mpu.giro_x;
                datos.giro_y  = mpu.giro_y;
                datos.giro_z  = mpu.giro_z;
                datos.magnitud_accel = sqrt(
                    mpu.accel_x * mpu.accel_x +
                    mpu.accel_y * mpu.accel_y +
                    mpu.accel_z * mpu.accel_z
                );
            } else {
                Serial.println("[TASK_SENSORES] MPU fallo — reiniciando...");
                mpu6050_init();
                datos.accel_x        = 0.0f;
                datos.accel_y        = 0.0f;
                datos.accel_z        = 0.0f;
                datos.magnitud_accel = 0.0f;
                datos.datos_validos  = false;
            }

            DatosINA219 ina;
            if (ina219_leer(ina)) {
                datos.voltaje_V    = ina.voltaje_bus;
                datos.corriente_mA = ina.corriente_mA;
                datos.potencia_mW  = ina.potencia_mW;
            }

            xSemaphoreGive(mutex_i2c);
        } else {
            Serial.println("[TASK_SENSORES] No se pudo tomar mutex I2C");
        }

        // ── Actualizar struct compartida ─────────────
// ── Actualizar struct compartida ─────────────
// Preserva campos calculados por otras tasks (rpm, nivel_celo, etc)
        if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(100)) == pdTRUE) {
            uint8_t   rpm_prev          = datos_compartidos.rpm;
            NivelCelo nivel_prev        = datos_compartidos.nivel_celo;
            bool      temp_alta_prev    = datos_compartidos.temp_alta;
            bool      monta_prev        = datos_compartidos.monta;
            uint8_t   ciclos_prev       = datos_compartidos.ciclos_actividad;

            datos_compartidos = datos;

            datos_compartidos.rpm              = rpm_prev;
            datos_compartidos.nivel_celo       = nivel_prev;
            datos_compartidos.temp_alta        = temp_alta_prev;
            datos_compartidos.monta            = monta_prev;
            datos_compartidos.ciclos_actividad = ciclos_prev;

            xSemaphoreGive(mutex_datos);
        }

        // ── Notificar al detector — xQueueOverwrite
        // nunca falla aunque la queue esté llena ──────

        bool monta_detectada = datos.fsr_voltaje >= UMBRAL_FSR_MONTA;

        Serial.printf("[TASK_SENSORES] Temp:%.2fC | Accel:%.2f m/s2 | Celo:%d | Consumo:%.1fmA | FSR:%.2fV | Monta:%s\n",
            datos.temperatura,
            datos.magnitud_accel,
            datos.nivel_celo,
            datos.corriente_mA,
            datos.fsr_voltaje,
            monta_detectada ? "SI" : "no");

        vTaskDelay(pdMS_TO_TICKS(INTERVALO_SENSORES));
    }
}