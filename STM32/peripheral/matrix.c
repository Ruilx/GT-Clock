#include <stdio.h>
#include <device.h>
#include <debug.h>
#include <system/clocks.h>
#include <system/systick.h>
#include "matrix.h"

static void matrix_init()
{
	// Configure GPIOs
	// PB3  SH SPI1: Alternate function output push-pull, 50MHz
	// PB4  OE:      Output push-pull, 50MHz
	// PB5  DE SPI1: Alternate function output push-pull, 50MHz
	// PA15 ST:      Output push-pull, 50MHz
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN_Msk | RCC_APB2ENR_IOPBEN_Msk | RCC_APB2ENR_AFIOEN_Msk;
	GPIOB->CRL = (GPIOB->CRL & ~(GPIO_CRL_CNF3_Msk | GPIO_CRL_MODE3_Msk |
				     GPIO_CRL_CNF4_Msk | GPIO_CRL_MODE4_Msk |
				     GPIO_CRL_CNF5_Msk | GPIO_CRL_MODE5_Msk)) |
		     ((0b10 << GPIO_CRL_CNF3_Pos) | (0b11 << GPIO_CRL_MODE3_Pos)) |
		     ((0b00 << GPIO_CRL_CNF4_Pos) | (0b11 << GPIO_CRL_MODE4_Pos)) |
		     ((0b10 << GPIO_CRL_CNF5_Pos) | (0b11 << GPIO_CRL_MODE5_Pos));
	GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF15_Msk | GPIO_CRH_CNF15_Msk)) |
		     ((0b00 << GPIO_CRH_CNF15_Pos) | (0b11 << GPIO_CRH_MODE15_Pos));
	// Also set JTAG to SWD only
	AFIO->MAPR |= AFIO_MAPR_SPI1_REMAP_Msk | (0b010 << AFIO_MAPR_SWJ_CFG_Pos);

	// Disable OE, ST = 0
	GPIOB->BSRR = GPIO_BSRR_BS4_Msk;
	GPIOA->BSRR = GPIO_BSRR_BR15_Msk;

	// Enable SPI1 clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN_Msk;
	// Configure SPI
	// SPI mode, I2S disabled
	SPI1->I2SCFGR = 0;
	// 1-line bidirectional, output enabled, CRC disabled, 8-bit data
	// SSM set NSS to 1, MSB first, SPI disabled, baud rate div by 4, master mode
	// CPOL = 0, CPHA = 0
	SPI1->CR1 = SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk | SPI_CR1_SSM_Msk | SPI_CR1_SSI_Msk |
		    (1 << SPI_CR1_BR_Pos) | SPI_CR1_MSTR_Msk;
	// SS output disable, TX & RX DMAs disabled
	SPI1->CR2 = 0;

	// Enable DMA clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN_Msk;

	// Enable SPI
	SPI1->CR1 |= SPI_CR1_SPE_Msk;

	// Test
#if 1
	static const uint8_t buf[] = {
		0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11111110,
		0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b11111011,
		0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b11101111,
		0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111111,
		0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b11111101,
		0b01111110, 0b01111110, 0b01111110, 0b01111110, 0b01111110, 0b11110111,
		0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b11011111,
		0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b01111111,
	};

	for (;;) {
		for (unsigned int i = 0; i < 8; i++) {
			for (unsigned int j = 0; j < 6; j++) {
				while (!(SPI1->SR & SPI_SR_TXE_Msk));
				SPI1->DR = buf[i * 6 + j];
			}
			// Wait for SPI to finish
			while (SPI1->SR & SPI_SR_BSY_Msk);

			// Disable OE
			GPIOB->BSRR = GPIO_BSRR_BS4_Msk;
			//for (unsigned int i = 512; i--;) __NOP();
			// Pulse ST
			GPIOA->BSRR = GPIO_BSRR_BS15_Msk;
			GPIOA->BSRR = GPIO_BSRR_BR15_Msk;
			// Enable OE
			GPIOB->BSRR = GPIO_BSRR_BR4_Msk;
		}
	}
#endif

	panic();

	printf(ESC_INIT "%lu\tmatrix: Initialisation done\n", systick_cnt());
}

INIT_HANDLER(&matrix_init);
