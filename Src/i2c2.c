/*
 * 
 * 
 * 
 * 
 */

#include "i2c2.h"

static uint8_t init = 0;

void I2C2_init(void) {
    /* Enable CLK */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    /* Enable/Config Pins PF0 and PF1 
        - Output open-drain, pull up
        - Alt func
        - GPIOx_AFR
    */
    // Port Mode = AF
    GPIOF->MODER |= (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);
    GPIOF->MODER &= ~(GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0);

    // Port Output Type = open-drain
    GPIOF->OTYPER |= (GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);

    // Output speed = High speed
    GPIOF->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR0_0 | GPIO_OSPEEDR_OSPEEDR1_0);
    GPIOF->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0_1 | GPIO_OSPEEDER_OSPEEDR1_1);

    // Pullup pulldown = pull up
    GPIOF->PUPDR |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0);
    GPIOF->PUPDR &= ~(GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1);

    // Alt Func Reg, AF4
    GPIOF->AFR[0] &= ~(GPIO_AFRH_AFRH0_Msk | GPIO_AFRH_AFRH1_Msk);
    GPIOF->AFR[0] |= (GPIO_AFRH_AFRH0_2 | GPIO_AFRH_AFRH1_2);

    /* 
    I2C peripheral clk must be configured and enabled in CC
    Then, I2C can be enabled by setting PE bit in I2C_CR1.
    Disabling by PE=0 results in I2C software reset, 30.4.5
    */
    // Disable I2C
    I2C2->CR1 &= ~I2C2_CR1_PE;

    /* I2C2 CR1 Bits */
    I2C2->CR1 &= ~(
        (0x01 << 17) // NOSTRETCH disable
        | (0x01 << 12) // Analog filter
        | (0x0F << 8) // Digital filter
    );

    I2C2->CR2 &= ~(
        (0x01 << 24) // RELOAD
        | (0x01 << 12) // HEAD10R
        | (0x01 << 11) // ADD10 7-bit mode 
    );

    /*
    Master Timing: 
    PRESC[3:0], SCLL[7:0], SCLH[7:0] in TIMINGR, I2C must be disabled
    Generated from CubeMX: 0x0010061A, bit 6 reserved
    */
    I2C2->TIMINGR |= 0x0010061A;
    I2C2->TIMINGR &= 0x01F0061A;

    // Enable I2C
    I2C2->CR1 |= I2C2_CR1_PE;

    init = 1;

}

void I2C2_SetDevice(uint8_t DAddress) {
    // Set SADD
    I2C2->CR2 &= ~I2C_CR2_SADD_Msk;
    I2C2->CR2 |= DAddress;
}

int I2C2_MemWrite(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout) {
    // Check init
    if (!init) {
        return 1;
    }
    // Check start bit
    if (I2C2->CR2 & (0x01 << 13)) {
        return 1;
    }

    /*
    Set Transfer Direction W, bit 10 = 0
    End type: bit 25 softend = 0
    Size, bit 16->
    Start bit 13 = 1
    */
    I2C2->CR2 &= ~(
        0x400 // DIR
        | I2C_CR2_NBYTES_Msk
    );
    I2C2->CR2 |= (
        (size + 1u) << I2C_CR2_NBYTES_Pos // Size
        | I2C_CR2_AUTOEND // END
        | I2C_CR2_START // Start
    );
    
    // Request write
    while (!(I2C2->ISR & I2C_ISR_TXE)); // TXE flag, add timeout?
    I2C2->TXDR = MemAddress;

    // Set message
    for (uint8_t i = 0; i < size;) {
        
        if (I2C2->ISR & 0x10) { // NACKF received
            I2C2->ICR |= 0x10; // Clear flag
            return 2;
        }

        while (!(I2C2->ISR & 0x02)); // TXIS flag, add timeout?
        I2C2->TXDR = data[i++];

    }

    //while (!(I2C2->ISR & I2C_ISR_TC)); // TC flag
    //while (!(I2C2->ISR & I2C_ISR_TXIS));
    //I2C2->CR2 |= I2C_CR2_STOP;

    while (!(I2C2->ISR & 0x20)); // STOP detect flag, add timeout?
    I2C2->ICR |= 0x20;

    return 0;

}

int I2C2_MemRead(uint8_t MemAddress, uint8_t* data, uint8_t size, uint32_t timeout) {
    // Check init
    if (!init) {
        return 1;
    }
    // Check start bit
    if (I2C2->CR2 & (0x01 << 13)) {
        return 1;
    }

    /*
    Set Transfer Direction W, bit 10 = 0
    End type: bit 25 softend = 0
    Size, bit 16->
    */

    I2C2->CR2 &= ~(
        I2C_CR2_NBYTES_Msk
        | 0x400 // DIR
        | (0x01 << 25) // END
    );

    I2C2->CR2 |= (
        0x10000 // Size
        | (0x01 << 13) // Start
    );

    // Request read
    while (!(I2C2->ISR & I2C_ISR_TXE)); // TXE flag, add timeout?
    I2C2->TXDR = MemAddress;

    // Wait TC flag, bit 6
    while(!(I2C2->ISR & 0x40));

    /*
    Set Transfer Direction R, bit 10 = 1
    Size, bit 16->
    End type: bit 25 autoend = 1
    Start bit 13 = 1
    */
    I2C2->CR2 &= ~I2C_CR2_NBYTES_Msk;
    I2C2->CR2 |= (
        0x400
        | (size << 16)
        | (0x01 << 25)
        | (0x01 << 13)
    );

    for (uint8_t i = 0; i < size;) {

        // Wait for read buffer to fill
        while(!(I2C2->ISR & I2C_ISR_RXNE));
        data[i++] = I2C2->RXDR;

    }

    while (!(I2C2->ISR & 0x20)); // STOP detect flag, add timeout?
    I2C2->ICR |= 0x20;

    return 0;
}
