#ifndef TASK_SD_H
#define TASK_SD_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "../tipos.h"

void task_sd(void *pvParameters);

#endif