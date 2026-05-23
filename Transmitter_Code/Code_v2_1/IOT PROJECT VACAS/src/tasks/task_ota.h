#ifndef TASK_OTA_H
#define TASK_OTA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void task_ota(void *pvParameters);

#endif