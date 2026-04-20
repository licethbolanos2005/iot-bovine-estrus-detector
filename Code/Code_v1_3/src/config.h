#ifndef CONFIG_H
#define CONFIG_H

// ─── Pines I2C externos ───────────────────────────────
#define PIN_I2C_SDA     41
#define PIN_I2C_SCL     42

// ─── Pin 1-Wire DS18B20 ───────────────────────────────
#define PIN_ONE_WIRE    4
#define DS18B20_DESCONECTADO   -127.0f

// ─── Pin ADC FSR ─────────────────────────────────────
#define PIN_FSR_ADC     2

// ─── Pines SPI SD Card ───────────────────────────────
#define PIN_SPI_MOSI    5
#define PIN_SPI_MISO    6
#define PIN_SPI_SCK     7
#define PIN_SD_CS       45

// ─── Pines LoRa SX1262 internos — NO tocar ───────────
#define LORA_SCK        9
#define LORA_MOSI       10
#define LORA_MISO       11
#define LORA_CS         8
#define LORA_RST        12
#define LORA_IRQ        14
#define LORA_BUSY       13

// ─── OLED interno — NO tocar ─────────────────────────
#define OLED_SDA        17
#define OLED_SCL        18
#define OLED_RST        21

// ─── Configuración general ───────────────────────────
#define SERIAL_BAUD       115200
#define LECTURA_DELAY_MS  5000

// ─── Dirección I2C INA219 ────────────────────────────
#define INA219_ADDR     0x40

// ─── Frecuencia LoRa ─────────────────────────────────
#define LORA_FREQ       915E6

#endif // CONFIG_H