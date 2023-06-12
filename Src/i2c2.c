/*
 * i2c2.c
 * -- for STM32F7
 * Author: Nick Fan
 * Date: 6/2023
 */

#include "i2c2.h"

static uint8_t init = false;


void I2C2_init(void) {
    // Enable CLK
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    // Port Mode: Alternate Function 
    GPIOF->MODER |= (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);
    GPIOF->MODER &= ~(GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0);

    // Port Output Type: Open-drain
    GPIOF->OTYPER |= (GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);

    // Output speed: High speed
    GPIOF->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR0_0 | GPIO_OSPEEDR_OSPEEDR1_0);
    GPIOF->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0_1 | GPIO_OSPEEDER_OSPEEDR1_1);

    // Pullup pulldown: Pull up
    GPIOF->PUPDR |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0);
    GPIOF->PUPDR &= ~(GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1);

    // Alt Func Reg: AF4 for I2C2
    GPIOF->AFR[0] &= ~(GPIO_AFRH_AFRH0_Msk | GPIO_AFRH_AFRH1_Msk);
    GPIOF->AFR[0] |= (GPIO_AFRH_AFRH0_2 | GPIO_AFRH_AFRH1_2);

    // Disable I2C
    I2C2->CR1 &= ~I2C_CR1_PE;

    /* I2C2 CR1 Bits */
    I2C2->CR1 &= ~(
        I2C_CR1_NOSTRETCH // NOSTRETCH disable
        | I2C_CR1_ANFOFF // Analog filter
        | I2C_CR1_DFN // Digital filter 
    );

    I2C2->CR2 &= ~(
        I2C_CR2_RELOAD // RELOAD
        | I2C_CR2_HEAD10R // HEAD10R
        | I2C_CR2_ADD10 // ADD10 7-bit mode 
    );

    // Master Timing
    I2C2->TIMINGR |= 0x0010061A;
    I2C2->TIMINGR &= 0x01F0061A;

    // Enable I2C
    I2C2->CR1 |= I2C_CR1_PE;

    // Set init true
    init = true;

}


void I2C2_SetDevice(uint8_t DAddress) {
    // Set SADD
    I2C2->CR2 &= ~I2C_CR2_SADD_Msk;
    I2C2->CR2 |= DAddress;
}


void I2C2_MemWrite(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout) {
    // Check init
    if (!init) {
        return /*1*/;
    }
    // Check start bit
    if (I2C2->CR2 & I2C_CR2_START) {
        return /*1*/;
    }

    /*
    Set Transfer Direction W, bit 10 = 0
    End type: bit 25 autoend = 1
    Size, shift left to bit 16
    Start bit 13 = 1
    */
    I2C2->CR2 &= ~(
        I2C_CR2_RD_WRN // Direction
        | I2C_CR2_NBYTES_Msk // Clear size
    );

    I2C2->CR2 |= (
        ((size + I2C_REQUEST_SIZE) << I2C_CR2_NBYTES_Pos) // Size
        | I2C_CR2_AUTOEND // Autoend
        | I2C_CR2_START // Start
    );
    
    // Request write
    while (!(I2C2->ISR & I2C_ISR_TXE)); // TXE flag, add timeout?
    I2C2->TXDR = MemAddress;

    // Set message
    for (uint8_t i = 0; i < size;) {
        
        if (I2C2->ISR & I2C_ISR_NACKF) { // NACKF received
            I2C2->ICR |= I2C_ICR_NACKCF; // Clear flag
            return /*2*/;
        }

        while (!(I2C2->ISR & I2C_ISR_TXIS)); // TXIS flag, add timeout?
        I2C2->TXDR = data[i++];

    }

    while (!(I2C2->ISR & I2C_ISR_STOPF)); // STOP detect flag, add timeout?
    I2C2->ICR |= I2C_ICR_STOPCF; // Clear stop flag

    return /*0*/;

}


void I2C2_MemRead(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout) {
    // Check init
    if (!init) {
        return /*1*/;
    }
    // Check start bit
    if (I2C2->CR2 & (I2C_CR2_START)) {
        return /*1*/;
    }

    /*
    Set Transfer Direction W, bit 10 = 0
    End type: bit 25 softend = 0
    Size, shift left to bit 16
    */

    I2C2->CR2 &= ~(
        I2C_CR2_NBYTES_Msk
        | I2C_CR2_RD_WRN // Direction
        | I2C_CR2_AUTOEND // Softend
    );

    I2C2->CR2 |= (
        I2C_REQUEST_SIZE << I2C_CR2_NBYTES_Pos // One Byte Size to send address
        | I2C_CR2_START // Start
    );

    // Request read
    while (!(I2C2->ISR & I2C_ISR_TXE)); // TXE flag, add timeout?
    I2C2->TXDR = MemAddress;

    // Wait TC flag, bit 6
    while(!(I2C2->ISR & I2C_ISR_TC));

    /*
    Set Transfer Direction R, bit 10 = 1
    Size, bit 16->
    End type: bit 25 autoend = 1
    Start bit 13 = 1
    */
    I2C2->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C2->CR2 |= (
        I2C_CR2_RD_WRN
        | size << I2C_CR2_NBYTES_Pos // Size
        | I2C_CR2_AUTOEND // Autoend
        | I2C_CR2_START // Start
    );

    for (uint8_t i = 0; i < size;) {

        // Wait for read buffer to fill
        while(!(I2C2->ISR & I2C_ISR_RXNE));
        data[i++] = I2C2->RXDR;

    }

    while (!(I2C2->ISR & I2C_ISR_STOPF)); // STOP detect flag, add timeout?
    I2C2->ICR |= I2C_ICR_STOPCF;

    return /*0*/;

}
