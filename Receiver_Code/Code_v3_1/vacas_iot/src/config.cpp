#include "config.h"

bool initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Error al montar — formateando...");
        return false;
    }
    Serial.println("[SPIFFS] Montado correctamente");
    return true;
}

String readConfig() {
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

bool saveConfig(const String& json) {
    // Escritura atómica: escribir en .tmp y luego renombrar
    String tmpPath = String(CONFIG_PATH) + ".tmp";

    File f = SPIFFS.open(tmpPath, "w");
    if (!f) {
        Serial.println("[CONFIG] Error abriendo archivo temporal");
        return false;
    }

    f.print(json);
    f.close();

    // Eliminar el original si existe
    if (SPIFFS.exists(CONFIG_PATH)) {
        SPIFFS.remove(CONFIG_PATH);
    }

    // Renombrar .tmp → config.json
    if (!SPIFFS.rename(tmpPath, CONFIG_PATH)) {
        Serial.println("[CONFIG] Error renombrando archivo temporal");
        return false;
    }

    Serial.println("[CONFIG] Guardado correctamente");
    return true;
}

bool validateJson(const String& json) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    return err == DeserializationError::Ok;
}

String getDefaultConfig() {
    return R"({
  "project": "Gateway LoRa — Deteccion de Celo Bovino",
  "version": "1.0.0",
  "device_id": "gateway_001",
  "network": {
    "wifi_ssid": "PARRA CARDONA",
    "wifi_password": "JuanParra2005",
    "ap_ssid": "Gateway_Vacas",
    "ap_password": "gateway123"
  },
  "lora": {
    "frecuencia": 915.0,
    "spreading_factor": 7,
    "bandwidth": 125.0,
    "coding_rate": 5,
    "sync_word": 18,
    "potencia_tx": 14
  },
  "alertas": {
    "temp_max": 39.5,
    "actividad_max": 12.0,
    "corriente_max": 150.0
  }
})";
}