#include "sensor_ds3232.h"
#include <RTClib.h>
#include <Wire.h>

static RTC_DS3231 rtc;

bool ds3232_init() {
    if (!rtc.begin()) return false;
    if (rtc.lostPower()) {
        // Sincroniza con hora de compilación si perdió energía
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return true;
}

bool ds3232_leer(DatosRTC &datos) {
    DateTime now = rtc.now();
    datos.anio    = now.year();
    datos.mes     = now.month();
    datos.dia     = now.day();
    datos.hora    = now.hour();
    datos.minuto  = now.minute();
    datos.segundo = now.second();
    return true;
}