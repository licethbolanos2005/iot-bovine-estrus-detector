#include "queues_globales.h"
#include "task_detector.h"
#include "../config.h"

void task_detector(void *pvParameters) {
    Serial.println("[TASK_DETECTOR] Iniciada");

    static uint8_t ciclos_actividad = 0;

    for (;;) {
        DatosNodo datos;

        if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(500)) != pdTRUE) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        datos = datos_compartidos;
        xSemaphoreGive(mutex_datos);

        if (!datos.datos_validos) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        bool act_alta  = datos.magnitud_accel >= UMBRAL_ACTIVIDAD_ALTA;
        bool act_media = datos.magnitud_accel >= UMBRAL_ACTIVIDAD_MEDIA;
        bool temp_alta = datos.temperatura    >= UMBRAL_TEMP_ELEVADA;
        bool monta     = datos.fsr_voltaje    >= UMBRAL_FSR_MONTA;

        if (act_alta) {
            if (ciclos_actividad < 255) ciclos_actividad++;
        } else {
            if (ciclos_actividad > 0)   ciclos_actividad--;
        }

        NivelCelo nivel = CELO_NO_DETECTADO;
        if (monta && (act_media || temp_alta))
            nivel = CELO_CONFIRMADO;
        else if (ciclos_actividad >= CICLOS_ACTIVIDAD_ALERTA && temp_alta)
            nivel = CELO_PROBABLE;
        else if (ciclos_actividad >= CICLOS_ACTIVIDAD_ALERTA)
            nivel = CELO_POSIBLE;
        else if (temp_alta || monta)
            nivel = CELO_POSIBLE;

        if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(200)) == pdTRUE) {
            datos_compartidos.nivel_celo       = nivel;
            datos_compartidos.temp_alta        = temp_alta;
            datos_compartidos.monta            = monta;
            datos_compartidos.ciclos_actividad = ciclos_actividad;
            xSemaphoreGive(mutex_datos);
        }

        if (nivel >= CELO_PROBABLE) {
            AlertaCelo alerta;
            alerta.nivel        = nivel;
            alerta.timestamp_ms = millis();
            snprintf(alerta.mensaje, sizeof(alerta.mensaje),
                "CELO %s", nivel == CELO_CONFIRMADO ? "CONFIRMADO" : "PROBABLE");
            xQueueSend(queue_alertas, &alerta, pdMS_TO_TICKS(100));
            Serial.printf("[ALERTA] %s\n", alerta.mensaje);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}