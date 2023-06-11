/* 
 * mpu6050.h
 * Author: Nick Fan
 * Date: 6/2023
 */

#ifndef MPU6050_H
#define MPU6050_H

// Includes
#include "stdint.h"

// Defines
#define MPU6050_I2C_ADDRESS             0x68 << 1

#define MPU6050_RESET                   0x00
#define MPU6050_RESET_PWR_MGMT_1        0x40
#define MPU6050_RESET_WHO_AM_I          0x68

#define MPU6050_FIFO_EN_TEMP            0x07
#define MPU6050_FIFO_EN_XG              0x06
#define MPU6050_FIFO_EN_YG              0x05
#define MPU6050_FIFO_EN_ZG              0x04
#define MPU6050_FIFO_EN_ACCEL           0x03

// Registers
#define MPU6050_REG_SELF_TEST_X         0x0D
#define MPU6050_REG_SELF_TEST_Y         0x0E
#define MPU6050_REG_SELF_TEST_Z         0x0F
#define MPU6050_REG_SELF_TEST_A         0x10
#define MPU6050_REG_SMPLRT_DIV          0x19
#define MPU6050_REG_CONFIG              0x1A
#define MPU6050_REG_GYRO_CONFIG         0x1B
#define MPU6050_REG_ACCEL_CONFIG        0x1C
#define MPU6050_REG_FIFO_EN             0x23
#define MPU6050_REG_INT_PIN_CFG         0x37
#define MPU6050_REG_INT_ENABLE          0x38
#define MPU6050_REG_INT_STATUS          0x3A
#define MPU6050_REG_ACCEL_XOUT_H        0x3B
#define MPU6050_REG_GYRO_XOUT_H         0x43
#define MPU6050_REG_SIGNAL_PATH_RESET   0x68
#define MPU6050_REG_USER_CTL            0x6A
#define MPU6050_REG_PWR_MGMT_1          0x6B
#define MPU6050_REG_PWR_MGMT_2          0x6C
#define MPU6050_REG_FIFO_COUNTH         0x72
#define MPU6050_REG_FIFO_COUNTL         0x73
#define MPU6050_REG_FIFO_R_W            0x74
#define MPU6050_REG_WHO_AM_I            0x75

// Function pointers
typedef void(*I2C_Init)();
typedef void(*I2C_MemAccess)(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout);
typedef MPU6050_Status(*MPU6050_Read_Single)(MPU6050_I2C hi2c, uint8_t *data);
typedef MPU6050_Status(*MPU6050_Read_Two)(MPU6050_I2C hi2c, uint8_t *accelData, uint8_t *gyroData);

typedef enum {
    MPU_OK = 0x01,
    MPU_ERROR = 0x00
} MPU6050_Status;

typedef struct {
    I2C_Init Init;
    I2C_MemAccess MemWrite;
    I2C_MemAccess MemRead;
} MPU6050_I2C;

// Function prototypes
MPU6050_Status MPU6050_init(MPU6050_I2C *hi2c);
MPU6050_Status MPU6050_ReadAccel(MPU6050_I2C *hi2c, uint8_t *accelData);
MPU6050_Status MPU6050_ReadGyro(MPU6050_I2C *hi2c, uint8_t *gyroData);
MPU6050_Status MPU6050_ReadAll(MPU6050_I2C *hi2c, uint8_t *accelData, uint8_t *gyroData);

#endif // MPU6050_H