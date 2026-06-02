#include "task_webserver.h"
#include "../gestores/gestor_webserver.h"

void task_webserver(void *pvParameters) {
    Serial.println("[TASK_WEBSERVER] Iniciada");

    for (;;) {
        webserver_handle();
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms — atiende clientes sin saturar CPU
    }
}