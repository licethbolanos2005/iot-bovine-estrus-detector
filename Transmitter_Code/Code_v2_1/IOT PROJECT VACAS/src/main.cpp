#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "config.h"
#include "tipos.h"
#include "sensors/sensor_ds18b20.h"
#include "sensors/sensor_mpu6050.h"
#include "sensors/sensor_ina219.h"
#include "sensors/sensor_ds3232.h"
#include "sensors/sensor_fsr.h"
#include "tasks/task_sensores.h"
#include "tasks/task_lora.h"
#include "tasks/task_sd.h"
#include "tasks/task_detector.h"
#include "gestores/gestor_nvs.h"
#include "tasks/task_ota.h"
#include "tasks/task_respiracion.h"

static void inicializar_sensores();

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(5000);
    Serial.println("=== Sistema bovino IoT arrancando ===");

    gestor_nvs_init();

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    inicializar_sensores();

    mutex_i2c   = xSemaphoreCreateMutex();
    mutex_datos = xSemaphoreCreateMutex();
    queue_datos   = xQueueCreate(5, sizeof(DatosNodo));
    queue_alertas = xQueueCreate(QUEUE_ALERTAS_SIZE, sizeof(AlertaCelo));

    if (!mutex_i2c || !mutex_datos || !queue_datos || !queue_alertas) {
        Serial.println("ERROR: No se pudieron crear recursos FreeRTOS");
        while(1);
    }

    BaseType_t ret;

    // ── Core 1 — sensores y detección (mismo core que Wire) ──
    ret = xTaskCreatePinnedToCore(task_sensores, "Sensores", 4096, NULL, 3, NULL, 1);
    Serial.printf("  task_sensores: %s\n", ret == pdPASS ? "OK" : "FALLO");

    ret = xTaskCreatePinnedToCore(task_detector, "Detector", 3072, NULL, 2, NULL, 1);
    Serial.printf("  task_detector: %s\n", ret == pdPASS ? "OK" : "FALLO");

    // ── Core 0 — comunicaciones (LoRa y SD no usan Wire) ─────
    ret = xTaskCreatePinnedToCore(task_lora, "LoRa", 8192, NULL, 1, NULL, 0);
    Serial.printf("  task_lora:     %s\n", ret == pdPASS ? "OK" : "FALLO");

    ret = xTaskCreatePinnedToCore(task_sd, "SD", 4096, NULL, 1, NULL, 0);
    Serial.printf("  task_sd:       %s\n", ret == pdPASS ? "OK" : "FALLO");

    ret = xTaskCreatePinnedToCore(task_ota, "OTA", 4096, NULL, 1, NULL, 0);
    Serial.printf("  task_ota:      %s\n", ret == pdPASS ? "OK" : "FALLO");

    ret = xTaskCreatePinnedToCore(task_respiracion, "Respiracion", 4096, NULL, 3, NULL, 1);
    Serial.printf("  task_respiracion: %s\n", ret == pdPASS ? "OK" : "FALLO");

    Serial.println("=== Tasks FreeRTOS iniciadas ===");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(10000));
}

static void inicializar_sensores() {
    Serial.println("Inicializando sensores...");
    Serial.print("  DS18B20 : "); Serial.println(ds18b20_init() ? "OK" : "ERROR");
    Serial.print("  MPU6050 : "); Serial.println(mpu6050_init() ? "OK" : "ERROR");
    Serial.print("  INA219  : "); Serial.println(ina219_init()  ? "OK" : "ERROR (no critico)");
    Serial.print("  DS3232  : "); Serial.println(ds3232_init()  ? "OK" : "ERROR");
    fsr_init();
    Serial.println("  FSR     : OK");
}