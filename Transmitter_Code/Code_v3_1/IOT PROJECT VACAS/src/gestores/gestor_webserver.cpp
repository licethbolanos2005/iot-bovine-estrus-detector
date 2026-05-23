#include "gestor_webserver.h"
#include "gestor_spiffs.h"
#include "gestor_wifi.h"
#include <WebServer.h>
#include <WiFi.h>
#include <Update.h>

static WebServer server(80);

// ─── HTML del panel ──────────────────────────────────
static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Collar Bovino - Panel de Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: sans-serif; background: #f0f2f5; padding: 20px; }
    h1 { color: #1a1a2e; margin-bottom: 6px; font-size: 1.4em; }
    .subtitle { color: #555; font-size: 0.9em; margin-bottom: 24px; }
    .card { background: white; border-radius: 10px; padding: 20px;
            margin-bottom: 16px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); }
    h2 { color: #333; font-size: 1em; margin-bottom: 12px; }
    textarea { width: 100%; height: 280px; font-family: monospace;
               font-size: 12px; border: 1px solid #ddd; border-radius: 6px;
               padding: 10px; resize: vertical; }
    button { background: #1976d2; color: white; border: none;
             padding: 10px 20px; border-radius: 6px; cursor: pointer;
             font-size: 14px; margin-top: 10px; margin-right: 6px; }
    button:hover { background: #1565c0; }
    button.danger { background: #d32f2f; }
    button.danger:hover { background: #b71c1c; }
    .status { margin-top: 10px; padding: 8px 12px; border-radius: 6px;
              font-size: 13px; display: none; }
    .ok  { background: #e8f5e9; color: #2e7d32; display: block; }
    .err { background: #ffebee; color: #c62828; display: block; }
    .info-grid { display: grid; grid-template-columns: 1fr 1fr;
                 gap: 10px; font-size: 13px; }
    .info-item { background: #f8f9fa; padding: 10px; border-radius: 6px; }
    .info-item strong { display: block; color: #888; font-size: 11px;
                        margin-bottom: 4px; }
    input[type=file] { display: block; margin: 10px 0; }
    progress { width: 100%; height: 20px; }
  </style>
</head>
<body>
  <h1>🐄 Collar Bovino - Panel de Control</h1>
  <p class="subtitle">Administracion del nodo sensor LoRa</p>

  <div class="card">
    <h2>📡 Estado del Sistema</h2>
    <div class="info-grid">
      <div class="info-item"><strong>IP Local</strong><span id="ip">Cargando...</span></div>
      <div class="info-item"><strong>WiFi</strong><span id="wifi">Cargando...</span></div>
      <div class="info-item"><strong>Uptime</strong><span id="uptime">Cargando...</span></div>
      <div class="info-item"><strong>Heap libre</strong><span id="heap">Cargando...</span></div>
    </div>
    <button onclick="loadStatus()">🔄 Actualizar</button>
  </div>

  <div class="card">
    <h2>⚙️ Configuracion (config.json)</h2>
    <textarea id="cfgEditor">Cargando...</textarea>
    <br>
    <button onclick="loadConfig()">📥 Cargar config</button>
    <button onclick="saveConfig()">💾 Guardar config</button>
    <button class="danger" onclick="rebootDevice()">♻ Reiniciar dispositivo</button>
    <div id="cfgStatus" class="status"></div>
  </div>

  <div class="card">
    <h2>🔄 Actualizacion OTA via Web</h2>
    <p style="font-size:12px;color:#666;margin-bottom:8px;">
      Sube tu firmware.bin (genera con PlatformIO: Build → buscar en .pio/build/...)
    </p>
    <input type="file" id="otaFile" accept=".bin">
    <progress id="otaProgress" value="0" max="100" style="display:none;"></progress>
    <br>
    <button onclick="uploadOta()">⬆ Subir firmware.bin</button>
    <div id="otaStatus" class="status"></div>
  </div>

<script>
function showStatus(id, msg, ok) {
  var el = document.getElementById(id);
  el.textContent = msg;
  el.className = 'status ' + (ok ? 'ok' : 'err');
}

function loadStatus() {
  fetch('/status').then(r => r.json()).then(d => {
    document.getElementById('ip').textContent     = d.ip     || '—';
    document.getElementById('wifi').textContent   = d.wifi   || '—';
    document.getElementById('uptime').textContent = d.uptime || '—';
    document.getElementById('heap').textContent   = d.heap   || '—';
  }).catch(e => console.error('Error estado:', e));
}

function loadConfig() {
  fetch('/config').then(r => r.text()).then(d => {
    document.getElementById('cfgEditor').value = d;
    showStatus('cfgStatus', 'Configuracion cargada', true);
  }).catch(() => showStatus('cfgStatus', 'Error al cargar', false));
}

function saveConfig() {
  var json = document.getElementById('cfgEditor').value;
  try { JSON.parse(json); } catch(e) {
    showStatus('cfgStatus', 'JSON invalido: ' + e.message, false);
    return;
  }
  fetch('/config', { method: 'POST',
    headers: { 'Content-Type': 'application/json' }, body: json })
  .then(r => r.json())
  .then(d => showStatus('cfgStatus',
    d.status === 'ok' ? 'Guardado correctamente' : 'Error',
    d.status === 'ok'))
  .catch(() => showStatus('cfgStatus', 'Error al guardar', false));
}

function rebootDevice() {
  if (!confirm('Reiniciar el dispositivo?')) return;
  fetch('/reboot', { method: 'POST' })
  .then(() => showStatus('cfgStatus', 'Reiniciando...', true));
}

function uploadOta() {
  var f = document.getElementById('otaFile').files[0];
  if (!f) { alert('Selecciona un archivo .bin'); return; }

  var bar = document.getElementById('otaProgress');
  bar.style.display = 'block';
  bar.value = 0;
  showStatus('otaStatus', 'Subiendo firmware...', true);

  var formData = new FormData();
  formData.append('firmware', f, f.name);

  var xhr = new XMLHttpRequest();
  xhr.open('POST', '/ota', true);

  xhr.upload.onprogress = function(e) {
    if (e.lengthComputable) {
      var pct = (e.loaded / e.total) * 100;
      bar.value = pct;
      showStatus('otaStatus', 'Subiendo ' + pct.toFixed(1) + '%', true);
    }
  };

  xhr.onload = function() {
    if (xhr.status === 200) {
      showStatus('otaStatus', 'OTA completo - reiniciando...', true);
    } else {
      showStatus('otaStatus', 'Error OTA: ' + xhr.responseText, false);
    }
  };

  xhr.onerror = function() {
    showStatus('otaStatus', 'Error de conexion', false);
  };

  xhr.send(formData);
}

loadStatus();
loadConfig();
setInterval(loadStatus, 10000);
</script>
</body>
</html>
)rawhtml";

// ─── Handlers básicos ─────────────────────────────────
static void handleIndex() {
    server.send_P(200, "text/html", INDEX_HTML);
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

// ─── Iniciar servidor ────────────────────────────────
void webserver_iniciar() {
    server.on("/",       HTTP_GET,  handleIndex);
    server.on("/config", HTTP_GET,  handleGetConfig);
    server.on("/config", HTTP_POST, handlePostConfig);
    server.on("/status", HTTP_GET,  handleGetStatus);
    server.on("/reboot", HTTP_POST, handleReboot);
    server.on("/ota",    HTTP_POST, handleOtaResponse, handleOtaUpload);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[WEB] Servidor HTTP iniciado en puerto 80");
}

void webserver_handle() {
    server.handleClient();
}