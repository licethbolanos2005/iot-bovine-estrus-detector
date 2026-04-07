#ifndef SENSOR_MPU6050_H
#define SENSOR_MPU6050_H

#include <Arduino.h>

struct DatosMPU6050 {
    float accel_x, accel_y, accel_z;   // m/s²
    float giro_x,  giro_y,  giro_z;    // rad/s
    float temperatura;                  // °C
};

bool mpu6050_init();
bool mpu6050_leer(DatosMPU6050 &datos);

#endif