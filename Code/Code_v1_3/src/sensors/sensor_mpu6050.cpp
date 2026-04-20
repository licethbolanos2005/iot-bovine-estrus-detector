#include "sensor_mpu6050.h"
#include "../config.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

static Adafruit_MPU6050 mpu;

bool mpu6050_init() {
    // AD0 conectado a 3.3V → dirección 0x69
    if (!mpu.begin(0x69)) return false;

    // ── Despertar el sensor explícitamente ──────────
    Wire.beginTransmission(0x69);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    delay(100);

    // ── Configurar rangos ────────────────────────────
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    delay(100);
    return true;
}

bool mpu6050_leer(DatosMPU6050 &datos) {
    sensors_event_t a, g, temp;
    if (!mpu.getEvent(&a, &g, &temp)) return false;

    datos.accel_x     = a.acceleration.x;
    datos.accel_y     = a.acceleration.y;
    datos.accel_z     = a.acceleration.z;
    datos.giro_x      = g.gyro.x;
    datos.giro_y      = g.gyro.y;
    datos.giro_z      = g.gyro.z;
    datos.temperatura = temp.temperature;
    return true;
}