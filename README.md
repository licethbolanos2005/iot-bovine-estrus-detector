# 🐄 IoT Bovine Estrus Detector

Prototipo IoT basado en tecnología **LoRa** para la detección temprana del celo en bovinos mediante el monitoreo continuo de variables fisiológicas y comportamentales.

> Proyecto integrador — Ingeniería Electrónica  
> Unidad Central del Valle del Cauca - Semestre 8
> Autores: Liceth Bolaños, Santiago Albarracin, Arlyn Cardozo, Samuel Guevara

---

## 📋 Descripción general

El sistema integra un nodo transmisor montado en un chaleco portador adaptado al animal, el cual captura temperatura corporal, actividad motora, frecuencia respiratoria y eventos de monta. Los datos son procesados por un **ESP32 LoRa V3 (ESP32-S3 + SX1262)**, almacenados localmente en MicroSD con marca temporal RTC y transmitidos de forma inalámbrica al nodo receptor, donde son visualizados en **Grafana** a través de **InfluxDB**.

---

## 📁 Estructura del repositorio

```
iot-bovine-estrus-detector/
│
├── Board/                  # Diseño final de la PCB (KiCad)
│   └── ...                 # Archivos .kicad_pcb, .kicad_sch, Gerbers
│
├── Box/                    # Diseño de la caja de protección (impresión 3D)
│   └── ...                 # Archivos .stl
│
├── Receiver_Code/          # Firmware del nodo receptor
│   └── ...                 # Código ESP32 — recepción LoRa, envío a InfluxDB
│
├── Schematic/              # Esquemático electrónico del sistema
│   └── ...                 # Archivos .kicad_sch
│
├── Transmitter_Code/       # Firmware del nodo transmisor
│   └── ...                 # Código ESP32 — sensores, LoRa, MicroSD, RTC, Web server
│
└── README.md
```

---

## ⚙️ Hardware utilizado

| Componente | Función |
|---|---|
| WiFi LoRa 32 V3 (ESP32-S3 + SX1262) | Unidad central de procesamiento y comunicación |
| DS18B20 | Medición de temperatura corporal |
| MPU6050 | Actividad motora y frecuencia respiratoria |
| FSR (Force Sensitive Resistor) | Detección de eventos de monta |
| INA219 | Monitoreo de consumo energético |
| DS3231 (RTC) | Registro temporal de datos |
| Módulo MicroSD TF | Almacenamiento local de datos |
| Batería LiPo 3.7V | Alimentación del sistema |

---

## 📡 Parámetros de comunicación LoRa

| Parámetro | Valor |
|---|---|
| Banda | 915 MHz (ISM) |
| Spreading Factor | 7 |
| Bandwidth | 125 kHz |
| Coding Rate | 5 |
| Sync Word | 0x12 |
| Potencia TX | 14 dBm |

---

## 🖥️ Arquitectura del sistema

```
[Sensores] → [ESP32 Transmisor] → [LoRa 915 MHz] → [ESP32 Receptor]
                    ↓                                        ↓
               [MicroSD + RTC]                        [InfluxDB → Grafana]
                    ↓
             [Servidor Web Embebido / OTA]
```

---

## 🚀 Primeros pasos

### Requisitos

- [Arduino IDE](https://www.arduino.cc/en/software) o [PlatformIO](https://platformio.org/)
- Placa: **Heltec WiFi LoRa 32 V3**
- Librerías necesarias:
  - `RadioLib`
  - `OneWire` + `DallasTemperature`
  - `MPU6050`
  - `RTClib`
  - `SD`
  - `INA219`
  - `ArduinoJson`
  - `ESPAsyncWebServer`

### Cargar firmware transmisor

1. Abre `Transmitter_Code/` en Arduino IDE o PlatformIO.
2. Configura los parámetros de red WiFi y LoRa en el archivo de configuración.
3. Compila y carga en la placa ESP32 del nodo transmisor.

### Cargar firmware receptor

1. Abre `Receiver_Code/` en Arduino IDE o PlatformIO.
2. Configura la dirección IP de InfluxDB y las credenciales de red.
3. Compila y carga en la placa ESP32 del nodo receptor.

---

## 📊 Visualización

Los datos son enviados a **InfluxDB** y visualizados en tiempo real mediante **Grafana**, donde se monitorean:

- Temperatura corporal (°C)
- Magnitud de aceleración (actividad motora)
- Frecuencia respiratoria
- Consumo energético (mA)
- Estado de celo (algoritmo de detección)
- Eventos de monta (FSR)
- Calidad del enlace LoRa (RSSI / SNR)

---

## 🌐 Servidor web embebido

El nodo transmisor implementa un servidor web accesible en la red local mediante:

```
http://boviot.local
```

Desde la interfaz web es posible:
- Visualizar variables fisiológicas en tiempo real
- Configurar parámetros de red WiFi
- Actualizar el firmware de forma inalámbrica (**OTA**)
