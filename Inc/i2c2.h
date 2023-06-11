/*
 * i2c2.h
 * -- for STM32F7
 * Author: Nick Fan
 * Date: 6/2023
 */


#ifndef F7_I2C2_H
#define F7_I2C2_H

#include "stm32f7xx.h"
#include "stdbool.h"

#define I2C_REQUEST_SIZE 0x1U << 0U

// Function prototypes

/*
 * @brief Initializes I2C2 peripheral
 */
void I2C2_init(void);

/* 
 * @brief Sets I2C2 Device Address
 * @param DAddress I2C Peripheral Device Address
 */
void I2C2_SetDevice(uint8_t DAddress);

/* 
 * @brief Performs a burst memory write in I2C2
 * @param MemAddress register address of peripheral to access
 * @param data pointer(s) to bytes to send
 * @param size the number of bytes to send
 * @param timeout the timeout in milliseconds
 * @retval The completion status
*/
int I2C2_MemWrite(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout);

/* 
 * @brief Performs a burst memory read in I2C2
 * @param MemAddress register address of peripheral to access
 * @param data pointer(s) for storing received data
 * @param size the number of bytes to read
 * @param timeout the timeout in milliseconds
 * @retval The completion status
*/
int I2C2_MemRead(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout);

#endif // F7_I2C2_H