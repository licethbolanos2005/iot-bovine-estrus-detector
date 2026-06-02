#ifndef QUEUES_GLOBALES_H
#define QUEUES_GLOBALES_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "../tipos.h"

// Declaraciones externas — definidas en task_sensores.cpp
extern QueueHandle_t     queue_datos;
extern QueueHandle_t     queue_alertas;
extern SemaphoreHandle_t mutex_i2c;
extern SemaphoreHandle_t mutex_datos;
extern DatosNodo datos_compartidos;

#endif