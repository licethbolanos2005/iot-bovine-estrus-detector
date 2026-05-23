#ifndef SENSOR_INA219_H
#define SENSOR_INA219_H

#include <Arduino.h>

struct DatosINA219 {
    float voltaje_bus;    // V
    float corriente_mA;   // mA
    float potencia_mW;    // mW
};

bool ina219_init();
bool ina219_leer(DatosINA219 &datos);

#endif