/* 
 * mpu6050.c
 * Author: Nick Fan
 * Date: 6/2023
 */

#include "i2c2.h"
#include "mpu6050.h"

#define GYRO_DIVIDER 32.8f
#define ACCEL_DIVIDER 4096.0f
#define I2C_DELAY 5000
#define MPU6050_SMPLRT_DIV_0 0x00
#define INIT_SMPLRT_DIV(data) hi2c->MemWrite(MPU6050_REG_SMPLRT_DIV, data, 1, I2C_DELAY) // Sample Rate Divider
#define INIT_CONFIG(data) hi2c->MemWrite(MPU6050_REG_CONFIG, data, 1, I2C_DELAY) // Config
#define INIT_GYRO_CONF(data) hi2c->MemWrite(MPU6050_REG_GYRO_CONFIG, data, 1, I2C_DELAY) // Gyro config
#define INIT_ACCEL_CONF(data) hi2c->MemWrite(MPU6050_REG_ACCEL_CONFIG, data, 1, I2C_DELAY) // Accel config
#define INIT_FIFO_EN(data) hi2c->MemWrite(MPU6050_REG_FIFO_EN, data, 1, I2C_DELAY) // Enable FIFO
#define INIT_USER_CTL(data) hi2c->MemWrite(MPU6050_REG_USER_CTL, data, 1, I2C_DELAY) // User Ctl setup
#define INIT_PWR_MNG1(data) hi2c->MemWrite(MPU6050_REG_PWR_MGMT_1, data, 1, I2C_DELAY) // Power Mng 1 setup

static uint8_t accelBuff[6];
static uint8_t gyroBuff[6];

MPU6050_Status MPU6050_init(MPU6050_I2C *hi2c) {
    uint8_t temp;
    hi2c->Init();
    hi2c->MemRead(MPU6050_REG_WHO_AM_I, &temp, 1, 5000);

    if (temp != MPU6050_I2C_ADDRESS >> 1) {
        return 1;
    }

    INIT_SMPLRT_DIV(&(uint8_t){MPU6050_SMPLRT_DIV_0});

    temp = 0x00 << 3 | 0x03u;
    INIT_CONFIG(&temp);

    temp = 0x02;
    INIT_GYRO_CONF(&temp);

    temp = 0x02;
    INIT_ACCEL_CONF(&temp);

    INIT_FIFO_EN(&(uint8_t){
        MPU6050_FIFO_EN_ACCEL | MPU6050_FIFO_EN_XG
        | MPU6050_FIFO_EN_YG | MPU6050_FIFO_EN_ZG
    });

    temp = 0x06;
    INIT_USER_CTL(&temp);

    temp = 0x01;
    INIT_PWR_MNG1(&temp);

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
