#include "gestor_webserver.h"
#include "gestor_spiffs.h"
#include "gestor_wifi.h"
#include <WebServer.h>
#include <WiFi.h>
#include <Update.h>
#include "../tasks/queues_globales.h"
#include <SPIFFS.h>

static WebServer server(80);

// ─── Handlers básicos ─────────────────────────────────
static void handleIndex() {
    File f = SPIFFS.open("/index.html", "r");
    if (!f) {
        server.send(500, "text/plain", "No se pudo abrir index.html");
        return;
    }
    server.streamFile(f, "text/html");
    f.close();
}

static void handleGetConfig() {
    String cfg = spiffs_leer_config();
    if (cfg == "") {
        server.send(500, "application/json", "{\"error\":\"No se pudo leer config\"}");
        return;
    }
    server.send(200, "application/json", cfg);
}

static void handlePostConfig() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Body vacio\"}");
        return;
    }

    String body = server.arg("plain");

    if (!spiffs_validar_json(body)) {
        server.send(400, "application/json", "{\"error\":\"JSON invalido\"}");
        return;
    }

    if (!spiffs_guardar_config(body)) {
        server.send(500, "application/json", "{\"error\":\"Error al guardar\"}");
        return;
    }

    server.send(200, "application/json", "{\"status\":\"ok\"}");
    Serial.println("[WEB] Configuracion actualizada via HTTP");
}

static void handleGetStatus() {
    String ip   = wifi_conectado() ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    String wifi = wifi_conectado() ? "Conectado (" + WiFi.SSID() + ")" : "Solo AP";

    unsigned long upSec = millis() / 1000;
    String uptime = String(upSec / 3600) + "h " +
                    String((upSec % 3600) / 60) + "m " +
                    String(upSec % 60) + "s";

    String json = "{";
    json += "\"ip\":\""     + ip + "\",";
    json += "\"wifi\":\""   + wifi + "\",";
    json += "\"uptime\":\"" + uptime + "\",";
    json += "\"heap\":\""   + String(ESP.getFreeHeap() / 1024) + " KB\"";
    json += "}";

    server.send(200, "application/json", json);
}

static void handleReboot() {
    server.send(200, "application/json", "{\"status\":\"rebooting\"}");
    Serial.println("[WEB] Reinicio solicitado");
    delay(500);
    ESP.restart();
}

// ─── OTA completo via web ────────────────────────────
static void handleOtaUpload() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA] Inicio upload: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("[OTA] Completo: %u bytes\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

static void handleOtaResponse() {
    bool exito = !Update.hasError();
    server.send(200, "application/json",
        exito ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}");
    if (exito) {
        Serial.println("[OTA] Reiniciando con nuevo firmware");
        delay(1000);
        ESP.restart();
    }
}

static void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

static void handleGetDatos() {
    if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(200)) != pdTRUE) {
        server.send(503, "application/json", "{\"error\":\"busy\"}");
        return;
    }
    DatosNodo d = datos_compartidos;
    xSemaphoreGive(mutex_datos);

    // Nivel celo a texto
    const char* niveles[] = {"No detectado", "Posible", "Probable", "Confirmado"};
    const char* nivel_str = (d.nivel_celo <= 3) ? niveles[d.nivel_celo] : "?";

    String json = "{";
    json += "\"temperatura\":"   + String(d.temperatura,    2) + ",";
    json += "\"rpm\":"           + String(d.rpm)               + ",";
    json += "\"magnitud_accel\":" + String(d.magnitud_accel, 2) + ",";
    json += "\"fsr_voltaje\":"   + String(d.fsr_voltaje,    2) + ",";
    json += "\"corriente_mA\":"  + String(d.corriente_mA,   1) + ",";
    json += "\"voltaje_V\":"     + String(d.voltaje_V,      2) + ",";
    json += "\"nivel_celo\":"    + String(d.nivel_celo)        + ",";
    json += "\"nivel_celo_str\":\"" + String(nivel_str)     + "\",";
    json += "\"monta\":"         + String(d.monta ? "true" : "false") + ",";
    json += "\"temp_alta\":"     + String(d.temp_alta ? "true" : "false") + ",";
    json += "\"accel_x\":"       + String(d.accel_x,        2) + ",";
    json += "\"accel_y\":"       + String(d.accel_y,        2) + ",";
    json += "\"accel_z\":"       + String(d.accel_z,        2) + ",";
    json += "\"uptime_s\":"      + String(millis() / 1000)     + ",";
    json += "\"heap_kb\":"       + String(ESP.getFreeHeap() / 1024);
    json += "}";

    server.send(200, "application/json", json);
}

// ─── MAC del dispositivo ─────────────────────────────
static void handleGetMac() {
    String mac_sta = WiFi.macAddress();
    String mac_ap  = WiFi.softAPmacAddress();

    String json = "{";
    json += "\"mac_sta\":\"" + mac_sta + "\",";
    json += "\"mac_ap\":\""  + mac_ap  + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

// ─── Información de la red WiFi conectada ────────────
static void handleGetWifiInfo() {
    String json = "{";

    if (WiFi.status() == WL_CONNECTED) {
        int32_t  rssi    = WiFi.RSSI();
        uint8_t  canal   = WiFi.channel();
        String   ssid    = WiFi.SSID();
        String   bssid   = WiFi.BSSIDstr();
        String   ip      = WiFi.localIP().toString();
        String   gateway = WiFi.gatewayIP().toString();
        String   mascara = WiFi.subnetMask().toString();

        // Calidad de señal en porcentaje
        int calidad = 0;
        if      (rssi >= -50) calidad = 100;
        else if (rssi <= -100) calidad = 0;
        else calidad = 2 * (rssi + 100);

        json += "\"conectado\":true,";
        json += "\"ssid\":\""    + ssid    + "\",";
        json += "\"bssid\":\""   + bssid   + "\",";
        json += "\"canal\":"     + String(canal)   + ",";
        json += "\"rssi\":"      + String(rssi)    + ",";
        json += "\"calidad\":"   + String(calidad) + ",";
        json += "\"ip\":\""      + ip      + "\",";
        json += "\"gateway\":\"" + gateway + "\",";
        json += "\"mascara\":\""  + mascara + "\"";
    } else {
        json += "\"conectado\":false";
    }

    json += "}";
    server.send(200, "application/json", json);
}

// ─── Scanner de redes WiFi ───────────────────────────
static void handleScanWifi() {
    server.sendHeader("Access-Control-Allow-Origin", "*");

    int n = WiFi.scanNetworks();

    String json = "[";
    for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        String seguridad = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Abierta" : "Segura";
        int calidad = 0;
        int rssi = WiFi.RSSI(i);
        if      (rssi >= -50)  calidad = 100;
        else if (rssi <= -100) calidad = 0;
        else calidad = 2 * (rssi + 100);

        json += "{";
        json += "\"ssid\":\""      + WiFi.SSID(i)        + "\",";
        json += "\"rssi\":"        + String(rssi)         + ",";
        json += "\"calidad\":"     + String(calidad)      + ",";
        json += "\"canal\":"       + String(WiFi.channel(i)) + ",";
        json += "\"seguridad\":\"" + seguridad            + "\"";
        json += "}";
    }
    json += "]";

    WiFi.scanDelete();
    server.send(200, "application/json", json);
}

// ─── Conectar a nueva red WiFi ───────────────────────
static void handleConectarWifi() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Body vacio\"}");
        return;
    }

    String body = server.arg("plain");
    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok) {
        server.send(400, "application/json", "{\"error\":\"JSON invalido\"}");
        return;
    }

    const char* ssid     = doc["ssid"];
    const char* password = doc["password"];

    if (!ssid || strlen(ssid) == 0) {
        server.send(400, "application/json", "{\"error\":\"SSID requerido\"}");
        return;
    }

    // Intentar conexión
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500);
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        String ip = WiFi.localIP().toString();

        // Actualizar config.json con las nuevas credenciales
        String cfg = spiffs_leer_config();
        if (cfg != "") {
            JsonDocument cfgDoc;
            deserializeJson(cfgDoc, cfg);
            cfgDoc["network"]["wifi_ssid"]     = ssid;
            cfgDoc["network"]["wifi_password"] = password;
            String cfgStr;
            serializeJsonPretty(cfgDoc, cfgStr);
            spiffs_guardar_config(cfgStr);
        }

        String json = "{\"status\":\"ok\",\"ip\":\"" + ip + "\"}";
        server.send(200, "application/json", json);
        Serial.printf("[WiFi] Conectado a %s — IP: %s\n", ssid, ip.c_str());
    } else {
        server.send(200, "application/json", "{\"status\":\"error\",\"msg\":\"No se pudo conectar\"}");
        Serial.printf("[WiFi] Fallo conexion a %s\n", ssid);
    }
}

static void handleChartJs() {
    File f = SPIFFS.open("/chart.js", "r");
    if (!f) {
        server.send(404, "text/plain", "chart.js no encontrado");
        return;
    }
    server.streamFile(f, "application/javascript");
    f.close();
}

// ─── Iniciar servidor ────────────────────────────────
void webserver_iniciar() {
    server.on("/",             HTTP_GET,  handleIndex);
    server.on("/config",       HTTP_GET,  handleGetConfig);
    server.on("/config",       HTTP_POST, handlePostConfig);
    server.on("/status",       HTTP_GET,  handleGetStatus);
    server.on("/reboot",       HTTP_POST, handleReboot);
    server.on("/datos",        HTTP_GET, handleGetDatos);
    server.on("/mac",          HTTP_GET,  handleGetMac);
    server.on("/wifiinfo",     HTTP_GET,  handleGetWifiInfo);
    server.on("/scanwifi",     HTTP_GET,  handleScanWifi);
    server.on("/conectarwifi", HTTP_POST, handleConectarWifi);
    server.on("/chart.js",     HTTP_GET, handleChartJs);
    server.on("/ota",          HTTP_POST, handleOtaResponse, handleOtaUpload);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[WEB] Servidor HTTP iniciado en puerto 80");
}

void webserver_handle() {
    server.handleClient();
}