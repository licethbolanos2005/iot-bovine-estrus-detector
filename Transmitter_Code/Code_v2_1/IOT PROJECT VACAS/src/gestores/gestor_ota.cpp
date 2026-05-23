#include "gestor_ota.h"
#include "../config.h"
#include <WiFi.h>
#include <ArduinoOTA.h>

static bool wifi_ok = false;
static bool ota_ok  = false;

bool gestor_ota_init() {
    Serial.println("[OTA] Conectando WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(OTA_HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Timeout 15 segundos
    uint32_t inicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[OTA] No se pudo conectar WiFi — OTA deshabilitado");
        wifi_ok = false;
        return false;
    }

    wifi_ok = true;
    Serial.print("[OTA] WiFi OK — IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("[OTA] Hostname: ");
    Serial.println(OTA_HOSTNAME);

    // ── Configurar ArduinoOTA ─────────────────────────
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        Serial.println("\n[OTA] === Inicio de actualizacion ===");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA] === Actualizacion completada ===");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progreso: %u%%\r", (progress * 100) / total);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] ERROR[%u]: ", error);
        switch (error) {
            case OTA_AUTH_ERROR:    Serial.println("Auth Failed");    break;
            case OTA_BEGIN_ERROR:   Serial.println("Begin Failed");   break;
            case OTA_CONNECT_ERROR: Serial.println("Connect Failed"); break;
            case OTA_RECEIVE_ERROR: Serial.println("Receive Failed"); break;
            case OTA_END_ERROR:     Serial.println("End Failed");     break;
            default:                Serial.println("Desconocido");
        }
    });

    ArduinoOTA.begin();
    ota_ok = true;
    Serial.println("[OTA] Servicio iniciado — listo para recibir firmware");
    return true;
}

void gestor_ota_handle() {
    if (ota_ok) {
        ArduinoOTA.handle();
    }
}

bool gestor_ota_wifi_conectado() {
    return wifi_ok && (WiFi.status() == WL_CONNECTED);
}