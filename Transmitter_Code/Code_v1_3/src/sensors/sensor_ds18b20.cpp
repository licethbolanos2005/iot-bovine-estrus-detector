#include "sensor_ds18b20.h"
#include "../config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(PIN_ONE_WIRE);
static DallasTemperature ds18b20(&oneWire);

bool ds18b20_init() {
    ds18b20.begin();
    return (ds18b20.getDeviceCount() > 0);
}

float ds18b20_leer_temperatura() {
    ds18b20.requestTemperatures();
    return ds18b20.getTempCByIndex(0);
}