#include "wifi_manager.h"

static bool s_connected = false;
static String s_ip = "";

bool startWifi(const String& configJson) {
    JsonDocument doc;
    deserializeJson(doc, configJson);

    const char* ssid     = doc["network"]["wifi_ssid"];
    const char* password = doc["network"]["wifi_password"];
    const char* ap_ssid  = doc["network"]["ap_ssid"]     | "Gateway_Vacas";
    const char* ap_pass  = doc["network"]["ap_password"]  | "gateway123";

    // Modo AP+STA simultáneo
    WiFi.mode(WIFI_AP_STA);

    // Configurar AP
    WiFi.softAP(ap_ssid, ap_pass);
    Serial.printf("[WiFi] AP iniciado: SSID=%s IP=%s\n",
        ap_ssid, WiFi.softAPIP().toString().c_str());

    // Intentar conectar a red local
    if (ssid && strlen(ssid) > 0) {
        Serial.printf("[WiFi] Conectando a %s...\n", ssid);
        WiFi.begin(ssid, password);

        int intentos = 0;
        while (WiFi.status() != WL_CONNECTED && intentos < 20) {
            delay(500);
            Serial.print(".");
            intentos++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            s_connected = true;
            s_ip = WiFi.localIP().toString();
            Serial.printf("[WiFi] Conectado — IP: %s\n", s_ip.c_str());
            return true;
        } else {
            Serial.println("[WiFi] No se pudo conectar — solo modo AP");
        }
    }

    s_connected = false;
    s_ip = WiFi.softAPIP().toString();
    return false;
}

String getIP() {
    return s_ip;
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}