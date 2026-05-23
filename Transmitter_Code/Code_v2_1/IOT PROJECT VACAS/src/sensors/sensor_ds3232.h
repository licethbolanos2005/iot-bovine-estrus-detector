#ifndef SENSOR_DS3232_H
#define SENSOR_DS3232_H

#include <Arduino.h>

void ds3232_forzar_recalibracion();

struct DatosRTC {
    uint16_t anio;
    uint8_t  mes, dia;
    uint8_t  hora, minuto, segundo;
};

bool ds3232_init();
bool ds3232_leer(DatosRTC &datos);

#endif