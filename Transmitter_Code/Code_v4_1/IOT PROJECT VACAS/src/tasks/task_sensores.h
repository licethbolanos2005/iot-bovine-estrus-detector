#ifndef TASK_SENSORES_H
#define TASK_SENSORES_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "queues_globales.h"
#include "../tipos.h"

void task_sensores(void *pvParameters);

#endif