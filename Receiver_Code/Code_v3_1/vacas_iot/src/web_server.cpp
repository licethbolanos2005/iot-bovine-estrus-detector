#include "web_server.h"
#include "config.h"
#include "wifi_manager.h"
#include <WebServer.h>
#include <WiFi.h>

static WebServer server(80);

// ─── Página web principal ────────────────────────────────────────────────────
static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Gateway — Detección de Celo Bovino</title>
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
             font-size: 14px; margin-top: 10px; }
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
  </style>
</head>
<body>
  <h1>🐄 Gateway — Detección de Celo Bovino</h1>
  <p class="subtitle">Panel de administración del nodo gateway LoRa</p>

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
    <h2>⚙️ Configuración (config.json)</h2>
    <textarea id="cfgEditor">Cargando...</textarea>
    <br>
    <button onclick="loadConfig()">📥 Cargar config</button>
    <button onclick="saveConfig()">💾 Guardar config</button>
    <div id="cfgStatus" class="status"></div>
  </div>

  <div class="card">
    <h2>🔄 Actualización OTA</h2>
    <input type="file" id="otaFile" accept=".bin">
    <button onclick="uploadOta()">⬆️ Subir firmware.bin</button>
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
    showStatus('cfgStatus', 'Configuración cargada', true);
  }).catch(() => showStatus('cfgStatus', 'Error al cargar', false));
}

function saveConfig() {
  var json = document.getElementById('cfgEditor').value;
  try { JSON.parse(json); } catch(e) {
    showStatus('cfgStatus', 'JSON inválido: ' + e.message, false);
    return;
  }
  fetch('/config', { method: 'POST',
    headers: { 'Content-Type': 'application/json' }, body: json })
  .then(r => r.json())
  .then(d => showStatus('cfgStatus',
    d.status === 'ok' ? 'Guardado correctamente' : 'Error: ' + JSON.stringify(d), 
    d.status === 'ok'))
  .catch(() => showStatus('cfgStatus', 'Error al guardar', false));
}

function uploadOta() {
  var f = document.getElementById('otaFile').files[0];
  if (!f) { alert('Selecciona un archivo .bin'); return; }
  showStatus('otaStatus', 'Subiendo firmware...', true);
  fetch('/ota', { method: 'POST',
    headers: { 'Content-Type': 'application/octet-stream' }, body: f })
  .then(r => r.json())
  .then(d => showStatus('otaStatus', 'OTA: ' + d.status + ' — reiniciando...', true))
  .catch(() => showStatus('otaStatus', 'Error en OTA', false));
}

// Cargar datos al abrir la página
loadStatus();
loadConfig();
setInterval(loadStatus, 10000);
</script>
</body>
</html>
)rawhtml";

// ─── Handlers ────────────────────────────────────────────────────────────────

void handleIndex() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleGetConfig() {
    String cfg = readConfig();
    if (cfg == "") {
        server.send(500, "application/json", "{\"error\":\"No se pudo leer config\"}");
        return;
    }
    server.send(200, "application/json", cfg);
}

void handlePostConfig() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Body vacío\"}");
        return;
    }

    String body = server.arg("plain");

    if (!validateJson(body)) {
        server.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
        return;
    }

    if (!saveConfig(body)) {
        server.send(500, "application/json", "{\"error\":\"Error al guardar\"}");
        return;
    }

    server.send(200, "application/json", "{\"status\":\"ok\"}");
    Serial.println("[WEB] Configuración actualizada via HTTP");
}

void handleGetStatus() {
    String ip   = isConnected() ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    String wifi = isConnected() ? "Conectado (" + WiFi.SSID() + ")" : "Solo AP";

    unsigned long upSec = millis() / 1000;
    String uptime = String(upSec / 3600) + "h " +
                    String((upSec % 3600) / 60) + "m " +
                    String(upSec % 60) + "s";

    String json = "{";
    json += "\"ip\":\"" + ip + "\",";
    json += "\"wifi\":\"" + wifi + "\",";
    json += "\"uptime\":\"" + uptime + "\",";
    json += "\"heap\":\"" + String(ESP.getFreeHeap() / 1024) + " KB\"";
    json += "}";

    server.send(200, "application/json", json);
}

void handleOta() {
    server.send(200, "application/json", "{\"status\":\"ota_received\"}");
    Serial.println("[WEB] OTA recibido — reiniciando...");
    delay(500);
    ESP.restart();
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

// ─── Inicialización ──────────────────────────────────────────────────────────

void startWebServer() {
    server.on("/",       HTTP_GET,  handleIndex);
    server.on("/config", HTTP_GET,  handleGetConfig);
    server.on("/config", HTTP_POST, handlePostConfig);
    server.on("/status", HTTP_GET,  handleGetStatus);
    server.on("/ota",    HTTP_POST, handleOta);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[WEB] Servidor HTTP iniciado en puerto 80");
}

void handleWebServer() {
    server.handleClient();
}