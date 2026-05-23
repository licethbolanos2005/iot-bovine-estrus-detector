#ifndef TASK_DETECTOR_H
#define TASK_DETECTOR_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "../tipos.h"

void task_detector(void *pvParameters);

#endif