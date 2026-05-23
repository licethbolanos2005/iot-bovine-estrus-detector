#include "sensor_mpu6050.h"
#include "../config.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

static Adafruit_MPU6050 mpu;

bool mpu6050_init() {
    return mpu.begin();
}

bool mpu6050_leer(DatosMPU6050 &datos) {
    sensors_event_t a, g, temp;
    if (!mpu.getEvent(&a, &g, &temp)) return false;

    datos.accel_x    = a.acceleration.x;
    datos.accel_y    = a.acceleration.y;
    datos.accel_z    = a.acceleration.z;
    datos.giro_x     = g.gyro.x;
    datos.giro_y     = g.gyro.y;
    datos.giro_z     = g.gyro.z;
    datos.temperatura = temp.temperature;
    return true;
}