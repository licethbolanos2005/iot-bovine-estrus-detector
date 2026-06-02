#ifndef CONFIG_H
#define CONFIG_H

#define FORZAR_RECALIBRACION_RTC  0

// ─── Pines I2C externos ───────────────────────────────
#define PIN_I2C_SDA         41
#define PIN_I2C_SCL         42

// ─── Pin 1-Wire DS18B20 ───────────────────────────────
#define PIN_ONE_WIRE        4
#define DS18B20_DESCONECTADO -127.0f

// ─── Pin ADC FSR ─────────────────────────────────────
#define PIN_FSR_ADC         2

// ─── Pines SPI SD Card ───────────────────────────────
#define PIN_SPI_MOSI        5
#define PIN_SPI_MISO        6
#define PIN_SPI_SCK         7
#define PIN_SD_CS           45

// ─── Pines LoRa SX1262 internos ──────────────────────
#define LORA_SCK            9
#define LORA_MOSI           10
#define LORA_MISO           11
#define LORA_CS             8
#define LORA_RST            12
#define LORA_IRQ            14
#define LORA_BUSY           13

// ─── Parámetros LoRa ─────────────────────────────────
#define LORA_FREQ           915.0
#define LORA_BW             125.0
#define LORA_SF             7
#define LORA_CR             5
#define LORA_SW             0x12
#define LORA_PWR            14
#define LORA_PREAMBLE       8

// ─── OLED interno ────────────────────────────────────
#define OLED_SDA            17
#define OLED_SCL            18
#define OLED_RST            21

// ─── Configuración general ───────────────────────────
#define SERIAL_BAUD         115200
#define DEVICE_ID           "collar_001"

// ─── Tiempos de las tasks (ms) ───────────────────────
#define INTERVALO_SENSORES  5000
#define INTERVALO_LORA      5000
#define INTERVALO_SD        10000

// ─── Tamaño queues FreeRTOS ──────────────────────────
#define QUEUE_DATOS_SIZE    5
#define QUEUE_ALERTAS_SIZE  5

// ─── Direcciones I2C ─────────────────────────────────
#define INA219_ADDR         0x40
#define MPU6050_ADDR        0x69

// ─── Umbrales detección celo ─────────────────────────
#define UMBRAL_ACTIVIDAD_ALTA   12.0f
#define UMBRAL_ACTIVIDAD_MEDIA  10.5f
#define UMBRAL_TEMP_ELEVADA     39.5f
#define UMBRAL_FSR_MONTA        2.0f
#define CICLOS_ACTIVIDAD_ALERTA 3

// ─── WiFi para OTA ────────────────────────────────────
#define WIFI_SSID       "PARRA CARDONA"
#define WIFI_PASSWORD   "JuanParra2005"
#define OTA_HOSTNAME    "collar_bovino_001"
#define OTA_PASSWORD    "ota_secret_123"

#endif // CONFIG_H