#ifndef TASK_LORA_H
#define TASK_LORA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "../tipos.h"

void task_lora(void *pvParameters);

#endif