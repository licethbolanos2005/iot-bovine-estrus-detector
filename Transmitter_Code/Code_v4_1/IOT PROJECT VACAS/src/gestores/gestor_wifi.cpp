#include "gestor_wifi.h"
#include <ESPmDNS.h> 

static bool   s_connected = false;
static String s_ip        = "";

bool wifi_iniciar(const String& configJson) {
    JsonDocument doc;
    deserializeJson(doc, configJson);

    const char* ssid     = doc["network"]["wifi_ssid"];
    const char* password = doc["network"]["wifi_password"];
    const char* ap_ssid  = doc["network"]["ap_ssid"]     | "Collar_Bovino";
    const char* ap_pass  = doc["network"]["ap_password"] | "collar123";

    // Modo AP+STA simultáneo
    WiFi.mode(WIFI_AP_STA);

    // Modo AP — siempre disponible
    WiFi.softAP(ap_ssid, ap_pass);
    Serial.printf("[WiFi] AP iniciado: SSID=%s IP=%s\n",
        ap_ssid, WiFi.softAPIP().toString().c_str());

    // Intentar conectar a WiFi local
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
            Serial.printf("[WiFi] Conectado - IP: %s\n", s_ip.c_str());
            return true;
        } else {
            Serial.println("[WiFi] No se pudo conectar — solo modo AP");
        }
    }

    s_connected = false;
    s_ip = WiFi.softAPIP().toString();
    return false;
}

String wifi_ip() { return s_ip; }
bool wifi_conectado() { return WiFi.status() == WL_CONNECTED; }

bool wifi_iniciar_mdns(const char* hostname) {
    if (!MDNS.begin(hostname)) {
        Serial.println("[mDNS] Error iniciando mDNS");
        return false;
    }

    // Anunciar el servicio HTTP
    MDNS.addService("http", "tcp", 80);

    Serial.printf("[mDNS] Disponible en: http://%s.local\n", hostname);
    return true;
}