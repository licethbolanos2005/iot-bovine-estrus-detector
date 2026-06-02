#include "gestor_spiffs.h"

bool spiffs_init() {
    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Error al montar — formateando...");
        return false;
    }
    Serial.println("[SPIFFS] Montado correctamente");
    return true;
}

String spiffs_leer_config() {
    if (!SPIFFS.exists(CONFIG_PATH)) {
        Serial.println("[CONFIG] No existe config.json");
        return "";
    }

    File f = SPIFFS.open(CONFIG_PATH, "r");
    if (!f) {
        Serial.println("[CONFIG] Error abriendo config.json");
        return "";
    }

    String content = f.readString();
    f.close();
    return content;
}

bool spiffs_guardar_config(const String& json) {
    String tmpPath = String(CONFIG_PATH) + ".tmp";

    File f = SPIFFS.open(tmpPath, "w");
    if (!f) {
        Serial.println("[CONFIG] Error abriendo archivo temporal");
        return false;
    }

    f.print(json);
    f.close();

    if (SPIFFS.exists(CONFIG_PATH)) {
        SPIFFS.remove(CONFIG_PATH);
    }

    if (!SPIFFS.rename(tmpPath, CONFIG_PATH)) {
        Serial.println("[CONFIG] Error renombrando archivo temporal");
        return false;
    }

    Serial.println("[CONFIG] Guardado correctamente");
    return true;
}

bool spiffs_validar_json(const String& json) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    return err == DeserializationError::Ok;
}

String spiffs_config_default() {
    return R"({
  "project": "Collar IoT - Deteccion de Celo Bovino",
  "version": "1.0.0",
  "device_id": "collar_001",
  "network": {
    "wifi_ssid": "PARRA CARDONA",
    "wifi_password": "JuanParra2005",
    "ap_ssid": "BOVIOT",
    "ap_password": "collar123"
  },
  "lora": {
    "frecuencia": 915.0,
    "spreading_factor": 7,
    "bandwidth": 125.0,
    "coding_rate": 5,
    "sync_word": 18,
    "potencia_tx": 14
  },
  "umbrales": {
    "actividad_alta": 12.0,
    "temp_elevada": 39.5,
    "fsr_monta": 2.0,
    "ciclos_alerta": 3
  }
})";
}