#include <Arduino.h>
#include <RadioLib.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"

// ─── Umbral FSR para evento de monta ───────────────────────────────────────
#define UMBRAL_FSR_MONTA  2.0f

// ─── OLED (pines internos Heltec V3) ───────────────────────────────────────
#define OLED_SDA  17
#define OLED_SCL  18
#define OLED_RST  21
SSD1306Wire display(0x3C, OLED_SDA, OLED_SCL);

// ─── Pines SX1262 internos del Heltec V3 ───────────────────────────────────
#define LORA_CS    8
#define LORA_IRQ   14
#define LORA_RST   12
#define LORA_BUSY  13

SX1262 radio = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_BUSY);

volatile bool receivedFlag = false;

void IRAM_ATTR onReceive() {
  receivedFlag = true;
}

// ─── Mapeo nivel de celo ────────────────────────────────────────────────────
const char* nivelCeloStr(int n) {
  switch (n) {
    case 0: return "No detectado";
    case 1: return "Posible";
    case 2: return "Probable";
    case 3: return "Confirmado";
    default: return "Desconocido";
  }
}

const char* nivelCeloCorto(int n) {
  switch (n) {
    case 0: return "NO";
    case 1: return "POSIBLE";
    case 2: return "PROBABLE";
    case 3: return "CONFIRM.";
    default: return "?";
  }
}

// ─── OLED: pantalla de espera ───────────────────────────────────────────────
void mostrarEsperando() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0,  "  GATEWAY LoRa V3");
  display.drawString(0, 16, "─────────────────");
  display.drawString(0, 30, "  Esperando datos");
  display.drawString(0, 44, "  del nodo...");
  display.display();
}

// ─── OLED: datos del nodo ───────────────────────────────────────────────────
void mostrarDatos(float temp, const char* celo_corto, float consumo,
                  float fsr, bool monta, float rssi) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0,  "GATEWAY V3   " + String(rssi, 0) + "dBm");
  display.drawString(0, 11, "─────────────────────");
  display.drawString(0, 22, "T:" + String(temp, 1) + "C  " +
                            "I:" + String(consumo, 1) + "mA");
  display.drawString(0, 33, "FSR:" + String(fsr, 2) + "V  " +
                            "Monta:" + String(monta ? "SI" : "no"));
  display.drawString(0, 44, "Celo: " + String(celo_corto));
  display.display();
}

// ─── OLED: pantalla de inicio con IP ───────────────────────────────────────
void mostrarInicio(String ip, bool connected) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0,  "  GATEWAY LoRa V3");
  display.drawString(0, 12, "─────────────────");
  display.drawString(0, 24, connected ? "WiFi: Conectado" : "WiFi: Solo AP");
  display.drawString(0, 36, "IP: " + ip);
  display.drawString(0, 48, "LoRa: Escuchando");
  display.display();
}

// ─── Setup ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicializar OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  Wire.begin(OLED_SDA, OLED_SCL);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.drawString(0, 0,  "  GATEWAY LoRa V3");
  display.drawString(0, 16, "  Iniciando...");
  display.display();

  // Inicializar SPIFFS
  if (!initSPIFFS()) {
    Serial.println("[ERROR] SPIFFS no disponible");
  }

  // Cargar configuración
  String cfg = readConfig();
  Serial.println("[DEBUG] Config leída:");
  Serial.println(cfg);
  if (cfg == "") {
    Serial.println("[CONFIG] No existe config.json — usando por defecto");
    cfg = getDefaultConfig();
    saveConfig(cfg);
  }
  Serial.println("[CONFIG] OK");

  // Iniciar WiFi
  bool connected = startWifi(cfg);
  String ip = getIP();
  Serial.println("[WEB] Accede en: http://" + ip);

  // Iniciar Web Server
  startWebServer();

  // Mostrar IP en OLED
  mostrarInicio(ip, connected);
  delay(2000);

  // Inicializar LoRa
  Serial.println("[GATEWAY] Iniciando receptor LoRa...");
  int state = radio.begin(915.0, 125.0, 7, 5, 0x12, 14, 8);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[ERROR] Fallo al iniciar radio: ");
    Serial.println(state);
    display.clear();
    display.drawString(0, 20, "ERROR radio:");
    display.drawString(0, 35, "codigo: " + String(state));
    display.display();
    while (true);
  }

  Serial.println("[GATEWAY] Radio OK. Escuchando...");
  radio.setPacketReceivedAction(onReceive);

  state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[ERROR] Fallo recepcion: ");
    Serial.println(state);
    while (true);
  }

  mostrarEsperando();
  Serial.println("[GATEWAY] Sistema listo");
}

// ─── Loop ───────────────────────────────────────────────────────────────────
void loop() {
  // Atender peticiones HTTP — debe llamarse siempre
  handleWebServer();

  // Procesar paquetes LoRa
  if (!receivedFlag) return;
  receivedFlag = false;

  String payload = "";
  int state = radio.readData(payload);

  if (state == RADIOLIB_ERR_NONE) {
    float rssi = radio.getRSSI();
    float snr  = radio.getSNR();

    Serial.println("─────────────────────────────────");
    Serial.print("[GATEWAY] RSSI : "); Serial.print(rssi); Serial.println(" dBm");
    Serial.print("[GATEWAY] SNR  : "); Serial.print(snr);  Serial.println(" dB");
    Serial.print("[GATEWAY] Bytes: "); Serial.println(payload.length());
    Serial.println("[GATEWAY] JSON recibido:");
    Serial.println(payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("[ERROR] JSON invalido: ");
      Serial.println(error.c_str());
      display.clear();
      display.drawString(0, 20, "JSON invalido");
      display.drawString(0, 35, "RSSI: " + String(rssi, 1));
      display.display();

    } else {
      const char* device_id  = doc["id"];
      float temperatura      = doc["s"]["t"];
      float consumo_mA       = doc["s"]["mA"];
      float voltaje_V        = doc["s"]["V"];
      float fsr_voltaje      = doc["s"]["fsr"];
      int   nivel_num        = doc["c"]["n"];
      float actividad        = doc["c"]["act"];
      bool  monta            = doc["c"]["m"];
      const char* nivel_celo = nivelCeloStr(nivel_num);

      bool monta_calc = fsr_voltaje >= UMBRAL_FSR_MONTA;

      Serial.println("[OK] ─── Datos recibidos ───");
      Serial.print("[OK] Device   : "); Serial.println(device_id);
      Serial.print("[OK] Temp     : "); Serial.print(temperatura); Serial.println(" C");
      Serial.print("[OK] Accel    : "); Serial.print(actividad);   Serial.println(" m/s2");
      Serial.print("[OK] Celo     : "); Serial.println(nivel_celo);
      Serial.print("[OK] Consumo  : "); Serial.print(consumo_mA);  Serial.println(" mA");
      Serial.print("[OK] Voltaje  : "); Serial.print(voltaje_V);   Serial.println(" V");
      Serial.print("[OK] FSR      : "); Serial.print(fsr_voltaje); Serial.println(" V");
      Serial.print("[OK] Monta    : "); Serial.println(monta || monta_calc ? "SI" : "no");

      mostrarDatos(temperatura, nivelCeloCorto(nivel_num),
                   consumo_mA, fsr_voltaje, monta || monta_calc, rssi);
    }

  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    Serial.println("[ERROR] CRC invalido — paquete corrupto");
  } else {
    Serial.print("[ERROR] Fallo lectura: ");
    Serial.println(state);
  }

  radio.startReceive();
}