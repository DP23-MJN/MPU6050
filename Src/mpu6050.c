/* 
 * mpu6050.c
 * Author: Nick Fan
 * Date: 6/2023
 */

#include "mpu6050.h"


static uint8_t accelBuff[6];
static uint8_t gyroBuff[6];

MPU6050_Status MPU6050_init(MPU6050_I2C *hi2c) {
    uint8_t address;
    
    // Initialize I2C
    hi2c->Init();

    // Check device address
    hi2c->MemRead(MPU6050_REG_WHO_AM_I, &address, 1, I2C_DELAY);

    if (address != MPU6050_I2C_ADDRESS >> 1) {
        return MPU_ERROR;
    }

    // Setup
    INIT_SMPLRT_DIV(&(uint8_t){MPU6050_SMPLRT_DIV_0});

    INIT_CONFIG(&(uint8_t){
        MPU6050_EXT_SYNC_SET_OFF << MPU6050_EXT_SYNC_POS 
        | MPU6050_DLPF_CFG_3
    });

    INIT_GYRO_CONF(&(uint8_t){
        MPU6050_FS_SEL_1000
        << MPU6050_FS_SEL_POS
    });

    INIT_ACCEL_CONF(&(uint8_t){
        MPU6050_AFS_SEL_8
        << MPU6050_AFS_SEL_POS
    });

    INIT_FIFO_EN(&(uint8_t){
        MPU6050_FIFO_EN_ACCEL | MPU6050_FIFO_EN_XG
        | MPU6050_FIFO_EN_YG | MPU6050_FIFO_EN_ZG
    });

    INIT_USER_CTL(&(uint8_t){MPU6050_USER_CTL_FIFO_EN});

    INIT_PWR_MNG1(&(uint8_t){MPU6050_CLKSEL_GYRO_X});

    return MPU_OK;
}

MPU6050_Status MPU6050_ReadAccel(MPU6050_I2C *hi2c, uint8_t *accelData) {
    hi2c->MemRead(MPU6050_REG_ACCEL_XOUT_H, accelBuff, 6, I2C_DELAY);
    accelData[0] = (int16_t) (accelBuff[0] << 8 | accelBuff[1]);
    accelData[1] = (int16_t) (accelBuff[2] << 8 | accelBuff[3]);
    accelData[2] = (int16_t) (accelBuff[4] << 8 | accelBuff[5]);
    /*for (uint8_t i = 0; i <= 2; ++i) {
        accelData[i] = (int16_t) (accelBuff[i] << 8 | accelBuff[++i]) / ACCEL_DIVIDER;
    }*/
}

MPU6050_Status MPU6050_ReadGyro(MPU6050_I2C *hi2c, uint8_t *gyroData) {
    hi2c->MemRead(MPU6050_REG_GYRO_XOUT_H, gyroBuff, 6, I2C_DELAY);
    gyroData[0] = (int16_t) (gyroBuff[0] << 8 | gyroBuff[1]);
    gyroData[1] = (int16_t) (gyroBuff[2] << 8 | gyroBuff[3]);
    gyroData[2] = (int16_t) (gyroBuff[4] << 8 | gyroBuff[5]);
}

MPU6050_Status MPU6050_ReadAll(MPU6050_I2C *hi2c, uint8_t *accelData, uint8_t *gyroData) {
    hi2c->MemRead(MPU6050_REG_ACCEL_XOUT_H, accelBuff, 6, I2C_DELAY);
    hi2c->MemRead(MPU6050_REG_GYRO_XOUT_H, gyroBuff, 6, I2C_DELAY);
    for (uint8_t i = 0; i <= 2; ++i) {
        accelData[i] = (int16_t) (accelBuff[2 * i] << 8 | accelBuff[2 * i + 1]) / ACCEL_DIVIDER;
        gyroData[i] = (int16_t) (gyroBuff[2 * i] << 8 | gyroBuff[2 * i + 1]) / GYRO_DIVIDER;
    }
}
