#include "gestor_json.h"
#include <ArduinoJson.h>
#include "../config.h"
#include "../sensors/detector_celo.h"

void gestor_json_serializar(const DatosNodo &datos,
                             char *buffer,
                             size_t buffer_size) {
    JsonDocument doc;

    // ── Metadata ──────────────────────────────────────
    doc["id"] = datos.device_id;

    char ts_buf[20];
    snprintf(ts_buf, sizeof(ts_buf), "%04d-%02d-%02dT%02d:%02d:%02d",
        datos.anio, datos.mes, datos.dia,
        datos.hora, datos.minuto, datos.segundo);
    doc["ts"] = ts_buf;

    // ── Sensores ──────────────────────────────────────
    JsonObject s = doc["s"].to<JsonObject>();
    s["t"]   = round(datos.temperatura  * 100) / 100.0;
    s["fsr"] = round(datos.fsr_voltaje  * 100) / 100.0;
    s["mA"]  = round(datos.corriente_mA * 10)  / 10.0;
    s["V"]   = round(datos.voltaje_V    * 100) / 100.0;
    s["rpm"] = datos.rpm;

    JsonObject a = s["a"].to<JsonObject>();
    a["x"] = round(datos.accel_x * 100) / 100.0;
    a["y"] = round(datos.accel_y * 100) / 100.0;
    a["z"] = round(datos.accel_z * 100) / 100.0;

    JsonObject g = s["g"].to<JsonObject>();
    g["x"] = round(datos.giro_x * 1000) / 1000.0;
    g["y"] = round(datos.giro_y * 1000) / 1000.0;
    g["z"] = round(datos.giro_z * 1000) / 1000.0;

    // ── Detección celo ────────────────────────────────
    JsonObject c = doc["c"].to<JsonObject>();
    c["n"]   = (uint8_t)datos.nivel_celo;
    c["act"] = round(datos.magnitud_accel * 100) / 100.0;
    c["ta"]  = datos.temp_alta;
    c["m"]   = datos.monta;

    size_t bytes = serializeJson(doc, buffer, buffer_size);

    Serial.printf("[JSON] %d bytes / %d max\n", bytes, buffer_size);
    if (bytes >= buffer_size - 1) {
        Serial.println("[JSON] ADVERTENCIA: posible truncado");
    }
}