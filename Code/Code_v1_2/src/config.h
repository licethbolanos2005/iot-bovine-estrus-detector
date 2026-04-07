#ifndef CONFIG_H
#define CONFIG_H

// ─── Pines I2C ───────────────────────────────────────
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// ─── Pin 1-Wire DS18B20 ───────────────────────────────
#define PIN_ONE_WIRE    4
// ─── Constante DS18B20 ───────────────────────────────
#define DS18B20_DESCONECTADO   -127.0f
// ─── Pin ADC FSR ─────────────────────────────────────
#define PIN_FSR_ADC     34

// ─── Pines SPI SD Card ───────────────────────────────
#define PIN_SPI_MOSI    23
#define PIN_SPI_MISO    19
#define PIN_SPI_SCK     18
#define PIN_SD_CS       15

// ─── Configuración general ───────────────────────────
#define SERIAL_BAUD     115200
#define LECTURA_DELAY_MS 5000

// ─── Dirección I2C INA219 ─────────────────────────────
// A0=GND, A1=GND → 0x40
#define INA219_ADDR     0x40

#endif // CONFIG_H