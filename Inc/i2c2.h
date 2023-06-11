/*
 * 
 * 
 * 
 * 
 */


#ifndef F7_I2C2_H
#define F7_I2C2_H

#include "stm32f7xx.h"

#define RCC_GPIOF_EN 0x01 << 5 // RCC_AHB1ENR
#define RCC_I2C2_EN 0x01 << 22 // RCC_APB1ENR
#define I2C2_CR1_PE 0x01
#define GPIOF_REG0_0 0x01
#define GPIOF_REG0_1 0x01 << 1
#define GPIOF_REG1_0 0x01 << 2
#define GPIOF_REG1_1 0x01 << 3
#define GPIOF_REG0 0x01
#define GPIOF_REG1 0x01 << 1

void I2C2_init(void);
void I2C2_SetDevice(uint8_t DAddress);
int I2C2_MemWrite(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout);
int I2C2_MemRead(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout);

#endif // F7_I2C2_H