/*
 * fmpi2c.c
 * for STM32F410
 * Author: Nick Fan
 * Date: 7/3/2023
 */

#include "fmpi2c.h"


void FMPI2C1_Init(void) {

	/* FMPI2C1 Using PB14 as SDA, PB15 as SCL */

	/* CLOCK SETUP: GPIOB, DMA, FMPI2C (I2C4) */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	// DMA

	// I2C
	RCC->APB1ENR |= RCC_APB1ENR_FMPI2C1EN;


	/* GPIO SETUP */

	// Set Mode: Alternate Func mode = 10
	GPIOB->MODER |= (GPIO_MODER_MODE14_1 | GPIO_MODER_MODE15_1);
	GPIOB->MODER &= ~(GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_0);

	// Set Output type: Output Open-drain = 1
	GPIOB->OTYPER |= (GPIO_OTYPER_OT14 | GPIO_OTYPER_OT15);

	// Set Output speed: Very high speed = 11
	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR14 | GPIO_OSPEEDER_OSPEEDR15);

	// Set Pullup/pulldown: Pull up = 01
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD14_0 | GPIO_PUPDR_PUPD15_0);
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD14_1 | GPIO_PUPDR_PUPD15_1);

	// Set Alternate Function Map: AF4 = 0100, for B14 and B15
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL14_Msk | GPIO_AFRH_AFSEL15_Msk); // Clear bits
	GPIOB->AFR[1] |= (GPIO_AFRH_AFSEL14_2| GPIO_AFRH_AFSEL15_2); // Set 0100


	/* DMA SETUP */

	/*
	 * I2C4 TX: Stream 1 Channel 2
	 * I2C4 RX: Stream 3 channel 1
	 */

	// Clear CHSEL bits, turn off double buffer, set PSIZE 8 bits, set DMA flow controller
	DMA1_Stream1->CR &= ~(DMA_SxCR_CHSEL_Msk | DMA_SxCR_DBM_Msk
						| DMA_SxCR_PSIZE_Msk | DMA_SxCR_PFCTRL_Msk);
	DMA1_Stream3->CR &= ~(DMA_SxCR_CHSEL_Msk | DMA_SxCR_DBM_Msk
						| DMA_SxCR_PSIZE_Msk | DMA_SxCR_PFCTRL_Msk);

	/* Select 010 for stream 1 channel 2 and 001 for stream 3 channel 1
	 * Enable memory increment mode, enable circular mode
	 */
	DMA1_Stream1->CR |= (DMA_SxCR_CHSEL_1 | DMA_SxCR_MINC | DMA_SxCR_CIRC);
	DMA1_Stream3->CR |= (DMA_SxCR_CHSEL_0 | DMA_SxCR_MINC | DMA_SxCR_CIRC);

	DMA1_Stream1->CR |= DMA_SxCR_DIR_0; // Set direction memory to peripheral
	DMA1_Stream3->CR &= ~DMA_SxCR_DIR_Msk; // Set direction peripheral to memory

	// Set direct mode enabled
	DMA1_Stream1->FCR &= ~(DMA_SxFCR_DMDIS_Msk);
	DMA1_Stream3->FCR &= ~(DMA_SxFCR_DMDIS_Msk);

	/* FMPI2C SETUP */

	// Disable FMPI2C
	FMPI2C1->CR1 &= ~(FMPI2C_CR1_PE);

	// Enable analog filter, disable digital noise filter, enable clock stretch
	FMPI2C1->CR1 &= ~(FMPI2C_CR1_ANFOFF | FMPI2C_CR1_DFN_Msk
					| FMPI2C_CR1_NOSTRETCH);

	// Set 7-bit addressing mode, Reload never used in API
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_ADD10 | FMPI2C_CR2_RELOAD);

	// Set TIMINGR PRESC, SCLH, SCLL
	FMPI2C1->TIMINGR &= CLEAR_REGISTER;
	FMPI2C1->TIMINGR |= CUBEMX_TIMINGR;

	// Enable FMPI2C
	FMPI2C1->CR1 |= FMPI2C_CR1_PE;

}

static void FMPI2C1_DMA_Enable() {
	// Enable I2C DMA transactions
	FMPI2C1->CR1 |= (FMPI2C_CR1_TXDMAEN | FMPI2C_CR1_RXDMAEN);

	// Stream enable
	DMA1_Stream1->CR |= DMA_SxCR_EN;
	DMA1_Stream3->CR |= DMA_SxCR_EN;
}

static void FMPI2C1_DMA_Disable() {
	// Disable I2C DMA transactions
	FMPI2C1->CR1 &= ~(FMPI2C_CR1_TXDMAEN | FMPI2C_CR1_RXDMAEN);

	// Stream disable
	DMA1_Stream1->CR &= ~DMA_SxCR_EN;
	DMA1_Stream3->CR &= ~DMA_SxCR_EN;

	// Wait for current transactions to complete and stream to be disabled
	while ((DMA1_Stream1->CR & DMA_SxCR_EN) || (DMA1_Stream3->CR & DMA_SXCR_EN));
}

void FMPI2C1_SetDevice(uint8_t deviceID) {
	// Set device address
	FMPI2C1->CR2 &= ~FMPI2C_CR2_SADD_Msk;
	FMPI2C1->CR2 |= (deviceID << FMPI2C_CR2_SADD_Pos);
}

SerialStatus FMPI2C1_MemWrite(uint8_t address, uint8_t *txdata, uint8_t size) {

	// Check if previous start has been sent already
	if (FMPI2C1->CR2 & FMPI2C_CR2_START) {
		return SERIAL_BUSY;
	}

	// Set write request, clear NBYTES value
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_RD_WRN | FMPI2C_CR2_NBYTES_Msk);

	// Set NBYTES transfer size, autoend mode, start bit - begins transfer
	FMPI2C1->CR2 |= (((size + I2C_ADDRESS_SIZE) << FMPI2C_CR2_NBYTES_Pos)
					| FMPI2C_CR2_AUTOEND | FMPI2C_CR2_START);


	// Check send ready, then send address
	while (!(FMPI2C1->ISR & FMPI2C_ISR_TXE));
	FMPI2C1->TXDR = address;

	// Loop to continue transmission for SIZE number of bytes
	for (uint8_t i = 0; i < size;) {

		// Check for NACKF
		if (FMPI2C1->ISR & FMPI2C_ICR_NACKCF) {
			return ERROR;
		}

		// Wait for transmit ready/ack
		while (!(FMPI2C1->ISR & FMPI2C_ISR_TXIS));
		FMPI2C1->TXDR = txdata[i++];

	}

	// Check for stop flag and clear stop flag
	while (!(FMPI2C1->ISR & FMPI2C_ISR_STOPF));
	FMPI2C1->ICR |= FMPI2C_ICR_STOPCF;

	return SERIAL_OK;

}

SerialStatus FMPI2C1_MemRead(uint8_t address, uint8_t *data, uint8_t size) {

	// Check if previous start has been sent already
	if (FMPI2C1->CR2 & FMPI2C_CR2_START) {
		return SERIAL_BUSY;
	}

	// Set write request, software end mode, clear NBYTES value
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_RD_WRN | FMPI2C_CR2_AUTOEND
					| FMPI2C_CR2_NBYTES_Msk);

	// Set NBYTES transfer size,
	FMPI2C1->CR2 |= ((I2C_ADDRESS_SIZE << FMPI2C_CR2_NBYTES_Pos)
					| FMPI2C_CR2_START);

	// Check ready to send, then send address
	while (!(FMPI2C1->ISR & FMPI2C_ISR_TXE));
	FMPI2C1->TXDR = address;

	// Software end - wait transfer complete flag
	while (!(FMPI2C1->ISR & FMPI2C_ISR_TC));

	// Clear NBYTES transfer size,
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_NBYTES_Msk);

	// Set NBYTES transfer size, read request, autoend mode, start bit - begins transfer
	FMPI2C1->CR2 |= ((size << FMPI2C_CR2_NBYTES_Pos) | FMPI2C_CR2_RD_WRN
					| FMPI2C_CR2_AUTOEND | FMPI2C_CR2_START);

	for (uint8_t i = 0; i < size;) {

		while(!(FMPI2C1->ISR & FMPI2C_ISR_RXNE));
		data[i++] = FMPI2C1->RXDR;
	}

	// Check for stop flag and clear stop flag
	while(!(FMPI2C1->ISR & FMPI2C_ISR_STOPF));
	FMPI2C1->ICR |= FMPI2C_ICR_STOPCF;

	return SERIAL_OK;

}

SerialStatus FMPI2C1_MemWrite_DMA(uint8_t address, uint8_t *txdata, uint8_t size) {

	// Check if previous start has been sent already
	if (FMPI2C1->CR2 & FMPI2C_CR2_START) {
		return SERIAL_BUSY;
	}

	// Disable, config, and enable DMA


	// Set write request, clear NBYTES value
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_RD_WRN | FMPI2C_CR2_NBYTES_Msk);

	// Set NBYTES transfer size, autoend mode, start bit - begins transfer
	FMPI2C1->CR2 |= (((size + I2C_ADDRESS_SIZE) << FMPI2C_CR2_NBYTES_Pos)
					| FMPI2C_CR2_AUTOEND | FMPI2C_CR2_START);



	return SERIAL_OK;

}

SerialStatus FMPI2C1_MemRead(uint8_t address, uint8_t *data, uint8_t size) {

	// Check if previous start has been sent already
	if (FMPI2C1->CR2 & FMPI2C_CR2_START) {
		return SERIAL_BUSY;
	}

	// Disable, config DMA


	// Set write request, software end mode, clear NBYTES value
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_RD_WRN | FMPI2C_CR2_AUTOEND
					| FMPI2C_CR2_NBYTES_Msk);

	// Set NBYTES transfer size,
	FMPI2C1->CR2 |= ((I2C_ADDRESS_SIZE << FMPI2C_CR2_NBYTES_Pos)
					| FMPI2C_CR2_START);

	// Check ready to send, then send address
	while (!(FMPI2C1->ISR & FMPI2C_ISR_TXE));
	FMPI2C1->TXDR = address;

	// Software end - wait transfer complete flag
	while (!(FMPI2C1->ISR & FMPI2C_ISR_TC));

	// Enable DMA


	// Clear NBYTES transfer size,
	FMPI2C1->CR2 &= ~(FMPI2C_CR2_NBYTES_Msk);

	// Set NBYTES transfer size, read request, autoend mode, start bit - begins transfer
	FMPI2C1->CR2 |= ((size << FMPI2C_CR2_NBYTES_Pos) | FMPI2C_CR2_RD_WRN
					| FMPI2C_CR2_AUTOEND | FMPI2C_CR2_START);


	return SERIAL_OK;

}
