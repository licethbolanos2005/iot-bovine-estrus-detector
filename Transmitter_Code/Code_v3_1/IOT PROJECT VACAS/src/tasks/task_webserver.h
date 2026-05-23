#ifndef TASK_WEBSERVER_H
#define TASK_WEBSERVER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void task_webserver(void *pvParameters);

#endif