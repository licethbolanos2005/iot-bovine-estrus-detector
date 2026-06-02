#include "sensor_ds3232.h"
#include <RTClib.h>
#include "../config.h"
#include <Wire.h>

static RTC_DS3231 rtc;

// ─── Configuración zona horaria Colombia (GMT-5) ──────
// El compilador usa la hora local del PC. Si tu PC está
// en hora Colombia, no necesitas offset.
#define OFFSET_HORARIO_HORAS  0   // Colombia ya es GMT-5

bool ds3232_init() {
    if (!rtc.begin(&Wire)) {
        Serial.println("[DS3231] No se pudo iniciar el modulo");
        return false;
    }

    bool necesita_calibrar = rtc.lostPower();

    #if FORZAR_RECALIBRACION_RTC == 1
        necesita_calibrar = true;
        Serial.println("[DS3231] Recalibración FORZADA por config.h");
    #endif

    if (necesita_calibrar) {
        Serial.println("[DS3231] Recalibrando...");
        DateTime ahora = DateTime(F(__DATE__), F(__TIME__));
        ahora = ahora + TimeSpan(0, OFFSET_HORARIO_HORAS, 0, 0);
        rtc.adjust(ahora);
        Serial.printf("[DS3231] Hora ajustada: %04d-%02d-%02d %02d:%02d:%02d\n",
            ahora.year(), ahora.month(), ahora.day(),
            ahora.hour(), ahora.minute(), ahora.second());
    } else {
        DateTime ahora = rtc.now();
        Serial.printf("[DS3231] Hora persistida OK: %04d-%02d-%02d %02d:%02d:%02d\n",
            ahora.year(), ahora.month(), ahora.day(),
            ahora.hour(), ahora.minute(), ahora.second());
    }

    Serial.printf("[DS3231] Temperatura interna: %.2f C\n", rtc.getTemperature());
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

// ─── Función opcional para forzar recalibración manual ──
void ds3232_forzar_recalibracion() {
    DateTime ahora = DateTime(F(__DATE__), F(__TIME__));
    ahora = ahora + TimeSpan(0, OFFSET_HORARIO_HORAS, 0, 0);
    rtc.adjust(ahora);
    Serial.println("[DS3231] Recalibración manual aplicada");
}
