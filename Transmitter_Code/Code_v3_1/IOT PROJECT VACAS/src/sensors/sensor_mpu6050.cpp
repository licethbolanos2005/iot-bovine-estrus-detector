#include "sensor_mpu6050.h"
#include "../config.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

static Adafruit_MPU6050 mpu;

bool mpu6050_init() {
    // Pasar &Wire explícito evita que la librería llame Wire.begin() de nuevo
    if (!mpu.begin(MPU6050_ADDR, &Wire)) return false;

    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    delay(100);
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