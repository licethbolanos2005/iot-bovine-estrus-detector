#include "task_ota.h"
#include "../config.h"
#include "../gestores/gestor_ota.h"

void task_ota(void *pvParameters) {
    Serial.println("[TASK_OTA] Iniciada");

    vTaskDelay(pdMS_TO_TICKS(2000));

    if (!gestor_ota_init()) {
        Serial.println("[TASK_OTA] OTA no disponible — task terminada");
        vTaskDelete(NULL);
        return;
    }

    for (;;) {
        gestor_ota_handle();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}