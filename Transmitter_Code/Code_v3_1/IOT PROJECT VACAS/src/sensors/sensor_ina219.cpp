#include "sensor_ina219.h"
#include "../config.h"
#include <Adafruit_INA219.h>
#include <Wire.h>

static Adafruit_INA219 ina219(INA219_ADDR);
static bool ina219_disponible = false;

bool ina219_init() {
    ina219_disponible = ina219.begin(&Wire);
    return ina219_disponible;
}

bool ina219_leer(DatosINA219 &datos) {
    if (!ina219_disponible) return false;
    datos.voltaje_bus  = ina219.getBusVoltage_V();
    datos.corriente_mA = ina219.getCurrent_mA();
    datos.potencia_mW  = ina219.getPower_mW();
    return true;
}