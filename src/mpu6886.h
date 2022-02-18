#pragma once
 
#include "stdint.h"
 
#define MPU6886_ADDRESS           0x68 
#define MPU6886_WHOAMI            0x75
#define MPU6886_ACCEL_INTEL_CTRL  0x69
#define MPU6886_SMPLRT_DIV        0x19
#define MPU6886_INT_PIN_CFG       0x37
#define MPU6886_INT_ENABLE        0x38
#define MPU6886_ACCEL_XOUT_H      0x3B
#define MPU6886_ACCEL_XOUT_L      0x3C
#define MPU6886_ACCEL_YOUT_H      0x3D
#define MPU6886_ACCEL_YOUT_L      0x3E
#define MPU6886_ACCEL_ZOUT_H      0x3F
#define MPU6886_ACCEL_ZOUT_L      0x40
 
#define MPU6886_TEMP_OUT_H        0x41
#define MPU6886_TEMP_OUT_L        0x42
 
#define MPU6886_GYRO_XOUT_H       0x43
#define MPU6886_GYRO_XOUT_L       0x44
#define MPU6886_GYRO_YOUT_H       0x45
#define MPU6886_GYRO_YOUT_L       0x46
#define MPU6886_GYRO_ZOUT_H       0x47
#define MPU6886_GYRO_ZOUT_L       0x48
 
#define MPU6886_USER_CTRL         0x6A
#define MPU6886_PWR_MGMT_1        0x6B
#define MPU6886_PWR_MGMT_2        0x6C
#define MPU6886_CONFIG            0x1A
#define MPU6886_GYRO_CONFIG       0x1B
#define MPU6886_ACCEL_CONFIG      0x1C
#define MPU6886_ACCEL_CONFIG2     0x1D
#define MPU6886_FIFO_EN           0x23
 
/* @[declare_mpu6886_acc_scale_t] */
typedef enum {
    MPU6886_AFS_2G = 0,
    MPU6886_AFS_4G,
    MPU6886_AFS_8G,
    MPU6886_AFS_16G
} acc_scale_t;
/* @[declare_mpu6886_acc_scale_t] */
 
/* @[declare_mpu6886_gyro_scale_t] */
typedef enum {
    MPU6886_GFS_250DPS = 0,
    MPU6886_GFS_500DPS,
    MPU6886_GFS_1000DPS,
    MPU6886_GFS_2000DPS
} gyro_scale_t;
/* @[declare_mpu6886_gyro_scale_t] */
 
/* @[declare_mpu6886_init] */
int MPU6886_Init(void);
/* @[declare_mpu6886_init] */
 
/* @[declare_mpu6886_getacceladc] */
void MPU6886_GetAccelAdc(int16_t *ax, int16_t *ay, int16_t *az);
/* @[declare_mpu6886_getacceladc] */
 
/* @[declare_mpu6886_getgyroadc] */
void MPU6886_GetGyroAdc(int16_t *gx, int16_t *gy, int16_t *gz);
/* @[declare_mpu6886_getgyroadc] */
 
/* @[declare_mpu6886_gettempadc] */
void MPU6886_GetTempAdc(int16_t *t);
/* @[declare_mpu6886_gettempadc] */
 
/* @[declare_mpu6886_getgyrores] */
float MPU6886_GetGyroRes(gyro_scale_t scale);
/* @[declare_mpu6886_getgyrores] */
 
/* @[declare_mpu6886_getaccres] */
float MPU6886_GetAccRes(acc_scale_t scale);
/* @[declare_mpu6886_getaccres] */
 
/* @[declare_mpu6886_setgyrofsr] */
void MPU6886_SetGyroFSR(gyro_scale_t scale);
/* @[declare_mpu6886_setgyrofsr] */
 
/* @[declare_mpu6886_setaccelfsr] */
void MPU6886_SetAccelFSR(acc_scale_t scale);
/* @[declare_mpu6886_setaccelfsr] */
 
/* @[declare_mpu6886_getacceldata] */
void MPU6886_GetAccelData(float *ax, float *ay, float *az);
/* @[declare_mpu6886_getacceldata] */
 
/* @[declare_mpu6886_getgyrodata] */
void MPU6886_GetGyroData(float *roll, float *yaw, float *pitch);
/* @[declare_mpu6886_getgyrodata] */
 
/* @[declare_mpu6886_gettempdata] */
void MPU6886_GetTempData(float *t);
/* @[declare_mpu6886_gettempdata] */