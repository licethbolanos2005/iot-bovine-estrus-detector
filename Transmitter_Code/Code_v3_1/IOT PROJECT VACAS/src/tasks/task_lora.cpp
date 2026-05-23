#include "queues_globales.h"
#include "task_lora.h"
#include "../config.h"
#include "../gestores/gestor_lora.h"
#include "../gestores/gestor_json.h"

void task_lora(void *pvParameters) {
    Serial.println("[TASK_LORA] Iniciada");

    vTaskDelay(pdMS_TO_TICKS(3000));

    bool lora_ok = gestor_lora_init();

    if (!lora_ok) {
        Serial.println("[TASK_LORA] LoRa fallo — reportando cada 10s");
        for (;;) {
            Serial.println("[TASK_LORA] ERROR: radio no inicializado");
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }

    Serial.println("[TASK_LORA] Radio listo, iniciando transmisiones");

    DatosNodo datos;
    char json_buffer[JSON_BUFFER_SIZE];

    for (;;) {
        Serial.println("[TASK_LORA] Ciclo iniciado");

        if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(500)) == pdTRUE) {
            datos = datos_compartidos;
            xSemaphoreGive(mutex_datos);

            if (datos.datos_validos) {
                gestor_json_serializar(datos, json_buffer, sizeof(json_buffer));

                size_t len = strlen(json_buffer);
                Serial.printf("[TASK_LORA] JSON (%d bytes): %s\n", len, json_buffer);

                if (len >= sizeof(json_buffer) - 1) {
                    Serial.println("[TASK_LORA] ADVERTENCIA: JSON truncado");
                }

                bool ok = gestor_lora_transmitir(json_buffer);
                Serial.printf("[TASK_LORA] TX %s\n", ok ? "OK" : "ERROR");

            } else {
                Serial.println("[TASK_LORA] Datos no validos, omitiendo TX");
            }

        } else {
            Serial.println("[TASK_LORA] No se pudo tomar mutex_datos");
        }

        vTaskDelay(pdMS_TO_TICKS(INTERVALO_LORA));
    }
}