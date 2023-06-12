/* 
 * mpu6050.h
 * Author: Nick Fan
 * Date: 6/2023
 */

#ifndef MPU6050_H
#define MPU6050_H

// Includes
#include <stdint.h>

// Device Address and Reset Values
#define MPU6050_I2C_ADDRESS             0x68 << 1
#define MPU6050_RESET                   0x00
#define MPU6050_RESET_PWR_MGMT_1        0x40
#define MPU6050_RESET_WHO_AM_I          0x68

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

// Settings
#define GYRO_DIVIDER                    32.8f // Gyro Divider for +-1000deg/sec
#define ACCEL_DIVIDER                   4096.0f // Accel Divider for +-8g
#define I2C_DELAY                       5000 // 5000 ms delay

#define MPU6050_FIFO_EN_TEMP            0x07 // FIFO EN Temperature
#define MPU6050_FIFO_EN_XG              0x06 // FIFO EN Gyro X
#define MPU6050_FIFO_EN_YG              0x05 // FIFO EN Gyro Y
#define MPU6050_FIFO_EN_ZG              0x04 // FIFO EN Gyro Z
#define MPU6050_FIFO_EN_ACCEL           0x03 // FIFO EN Accel
#define MPU6050_SMPLRT_DIV_0            0x00 // Sample rate divider = 0
#define MPU6050_EXT_SYNC_SET_OFF        0x00 // CONFIG Ext Sync Set Off
#define MPU6050_EXT_SYNC_POS            3 // CONFIG Ext Sync Position
#define MPU6050_DLPF_CFG_3              0x03 // CONFIG Digital Low Pass Filter Set 3
#define MPU6050_FS_SEL_1000             0x02 // GYRO CONFIG FS_SEL +-1000deg/sec
#define MPU6050_FS_SEL_POS              3 // GYRO CONFIG FS_SEL Position
#define MPU6050_AFS_SEL_8               0x02 // ACCEL CONFIG AFS_SEL +-8g
#define MPU6050_AFS_SEL_POS             3 // ACCEL CONFIG AFS_SEL Position
#define MPU6050_USER_CTL_FIFO_EN        0x01 << 6 // USER CTL FIFO EN
#define MPU6050_CLKSEL_GYRO_X           0x01 // PWR MGMT1 CLKSEL GYRO X-Axis

#define INIT_SMPLRT_DIV(data)           hi2c->MemWrite(MPU6050_REG_SMPLRT_DIV, data, 1, I2C_DELAY) // Sample Rate Divider
#define INIT_CONFIG(data)               hi2c->MemWrite(MPU6050_REG_CONFIG, data, 1, I2C_DELAY) // Config
#define INIT_GYRO_CONF(data)            hi2c->MemWrite(MPU6050_REG_GYRO_CONFIG, data, 1, I2C_DELAY) // Gyro config
#define INIT_ACCEL_CONF(data)           hi2c->MemWrite(MPU6050_REG_ACCEL_CONFIG, data, 1, I2C_DELAY) // Accel config
#define INIT_FIFO_EN(data)              hi2c->MemWrite(MPU6050_REG_FIFO_EN, data, 1, I2C_DELAY) // Enable FIFO
#define INIT_USER_CTL(data)             hi2c->MemWrite(MPU6050_REG_USER_CTL, data, 1, I2C_DELAY) // User Ctl setup
#define INIT_PWR_MNG1(data)             hi2c->MemWrite(MPU6050_REG_PWR_MGMT_1, data, 1, I2C_DELAY) // Power Mng 1 setup

// Enums
typedef enum {
    MPU_OK = 0x01,
    MPU_ERROR = 0x00
} MPU6050_Status;

// Function pointers
typedef void(*I2C_Init)();
typedef void(*I2C_MemAccess)(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout);

// Structs
typedef struct {
    I2C_Init Init;
    I2C_MemAccess MemWrite;
    I2C_MemAccess MemRead;
} MPU6050_I2C;

// Function Pointers
typedef MPU6050_Status(*MPU6050_Read_Single)(MPU6050_I2C *hi2c, float *data);
typedef MPU6050_Status(*MPU6050_Read_Two)(MPU6050_I2C *hi2c, float *accelData, float *gyroData);


// Function prototypes
/* 
 * @brief Initializes MPU6050 on I2C
 * @param hi2c The I2C handler to initialize with
 * @retval The completion status
*/
MPU6050_Status MPU6050_init(MPU6050_I2C *hi2c);

/* 
 * @brief Reads Accelerometer data from MPU6050
 * @param hi2c The I2C handler to read with
 * @param accelData The buffer to store accel data, minimum size 3
 * @retval The completion status
*/
MPU6050_Status MPU6050_ReadAccel(MPU6050_I2C *hi2c, float *accelData);

/* 
 * @brief Reads Gyroscope data from MPU6050
 * @param hi2c The I2C handler to read with
 * @param gyroData The buffer to store gyro data, minimum size 3
 * @retval The completion status
*/
MPU6050_Status MPU6050_ReadGyro(MPU6050_I2C *hi2c, float *gyroData);

/* 
 * @brief Initializes MPU6050 on I2C
 * @param hi2c The I2C handler to initialize with
 * @param accelData The buffer to store accel data, minimum size 3
 * @param gyroData The buffer to store gyro data, minimum size 3
 * @retval The completion status
*/
MPU6050_Status MPU6050_ReadAll(MPU6050_I2C *hi2c, float *accelData, float *gyroData);

#endif // MPU6050_H
