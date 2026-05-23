#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "sensors/sensor_ds18b20.h"
#include "sensors/sensor_mpu6050.h"
#include "sensors/sensor_ina219.h"
#include "sensors/sensor_ds3232.h"
#include "sensors/sensor_fsr.h"
#include "sensors/detector_celo.h"

static void inicializar_sensores();
static void leer_e_imprimir_sensores();
static void imprimir_separador();
static void imprimir_alerta_celo(const ResultadoCelo &res);

// ─── Setup ────────────────────────────────────────────
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(2000);
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    detector_celo_init();
    inicializar_sensores();
    Serial.println("=== Sistema de monitoreo bovino listo ===\n");
}

// ─── Loop ─────────────────────────────────────────────
void loop() {
    leer_e_imprimir_sensores();
    delay(LECTURA_DELAY_MS);
}

// ─── Inicialización ───────────────────────────────────
static void inicializar_sensores() {
    Serial.println("Inicializando sensores...");

    Serial.print("  DS18B20  : ");
    Serial.println(ds18b20_init() ? "OK" : "ERROR");

    Serial.print("  MPU6050  : ");
    Serial.println(mpu6050_init() ? "OK" : "ERROR");

    Serial.print("  INA219   : ");
    Serial.println(ina219_init()  ? "OK" : "ERROR (no critico)");

    Serial.print("  DS3232   : ");
    Serial.println(ds3232_init()  ? "OK" : "ERROR");

    fsr_init();
    Serial.println("  FSR      : OK");

    imprimir_separador();
}

// ─── Lectura principal ────────────────────────────────
static void leer_e_imprimir_sensores() {
    imprimir_separador();

    // ── RTC ─────────────────────────────────────────
    DatosRTC rtc;
    if (ds3232_leer(rtc)) {
        Serial.printf("[RTC] %04d/%02d/%02d  %02d:%02d:%02d\n",
            rtc.anio, rtc.mes, rtc.dia,
            rtc.hora, rtc.minuto, rtc.segundo);
    } else {
        Serial.println("[RTC] Error");
    }

    // ── DS18B20 ─────────────────────────────────────
    float temp = ds18b20_leer_temperatura();
    if (temp != DS18B20_DESCONECTADO) {
        Serial.printf("[DS18B20] Temp: %.2f C\n", temp);
    } else {
        Serial.println("[DS18B20] Desconectado");
        temp = 0.0f;
    }

    // ── MPU6050 ─────────────────────────────────────
    DatosMPU6050 mpu;
    if (mpu6050_leer(mpu)) {
        Serial.printf("[MPU6050] Accel X:%.2f Y:%.2f Z:%.2f m/s2\n",
            mpu.accel_x, mpu.accel_y, mpu.accel_z);
        Serial.printf("[MPU6050] Giro  X:%.2f Y:%.2f Z:%.2f rad/s\n",
            mpu.giro_x, mpu.giro_y, mpu.giro_z);
    } else {
        Serial.println("[MPU6050] Error");
    }

    // ── INA219 ──────────────────────────────────────
    DatosINA219 ina;
    if (ina219_leer(ina)) {
        Serial.printf("[INA219]  %.2fV | %.2fmA | %.2fmW\n",
            ina.voltaje_bus, ina.corriente_mA, ina.potencia_mW);
    }

    // ── FSR ─────────────────────────────────────────
    float voltaje_fsr = fsr_leer_voltaje();
    Serial.printf("[FSR]     Raw:%d  Volt:%.3fV\n",
        fsr_leer_raw(), voltaje_fsr);

    // ── Detección de celo ────────────────────────────
    ResultadoCelo resultado;
    detector_celo_analizar(mpu, temp, voltaje_fsr, resultado);
    imprimir_alerta_celo(resultado);
}

// ─── Impresión de alerta ──────────────────────────────
static void imprimir_alerta_celo(const ResultadoCelo &res) {
    Serial.println();
    Serial.printf("[CELO] Estado   : %s\n",
        detector_celo_nivel_str(res.nivel));
    Serial.printf("[CELO] Actividad: %.2f m/s2 (ciclos altos: %d)\n",
        res.magnitud_accel, res.ciclos_actividad_alta);
    Serial.printf("[CELO] Temp alta: %s | Monta: %s\n",
        res.temperatura_elevada ? "SI" : "no",
        res.evento_monta        ? "SI" : "no");

    // Alerta visual si hay celo probable o confirmado
    if (res.nivel >= CELO_PROBABLE) {
        Serial.println();
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        Serial.printf ("!! ALERTA CELO: %-23s !!\n",
            detector_celo_nivel_str(res.nivel));
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    Serial.println();
}

static void imprimir_separador() {
    Serial.println("------------------------------------------");
}