#include <stdio.h>
#include <string.h>
#include <device.h>
#include <debug.h>
#include <system/irq.h>
#include <system/clocks.h>
#include <system/systick.h>
#include "matrix.h"

#define PANELS	5
#define LINES	8
#define GSCALE	32
#define FPS	480

#define LPS	(FPS * LINES * GSCALE)
#define PPS	(LPS * (PANELS + 1) * 8)

static struct {
	//uint8_t (*buf)[PANELS + 1];
	uint8_t buf[LINES * GSCALE][PANELS + 1];
	unsigned int ridx;
} buf = {0};

static const uint8_t buffer[LINES * 2][PANELS + 1] = {
	{0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11111110},
	{0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11010111, 0b11111110},
	{0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b11111101},
	{0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111111, 0b11111101},
	{0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b11111011},
	{0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b01111011, 0b11111011},
	{0b01111110, 0b01111110, 0b01111110, 0b01111110, 0b01111110, 0b11110111},
	{0b01111110, 0b01111110, 0b01111110, 0b01111110, 0b01111111, 0b11110111},
	{0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b11101111},
	{0b01011010, 0b01011010, 0b01011010, 0b01011010, 0b01111011, 0b11101111},
	{0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b11011111},
	{0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01110111, 0b11011111},
	{0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111111},
	{0b10111101, 0b10111101, 0b10111101, 0b10111101, 0b10111111, 0b10111111},
	{0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b01111111},
	{0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11010111, 0b01111111},
};

static inline void matrix_buf_init(const uint8_t pbuf[][PANELS + 1]);
static inline void matrix_line();

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
	// SS output disable, TX DMA enabled, RX DMA disabled
	SPI1->CR2 = SPI_CR2_TXDMAEN_Msk;
	// Enable SPI
	SPI1->CR1 |= SPI_CR1_SPE_Msk;

	// Enable DMA clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN_Msk;
	// Configure DMA1 Channel 3 for SPI1_TX
	// Disable stream
	DMA1_Channel3->CCR = 0;
	// Memory to peripheral, 8bit -> 8bit, burst of 4 beats, high priority
	// Memory increment, peripheral not increment, circular disabled
	DMA1_Channel3->CCR = (0b10 << DMA_CCR_PL_Pos) | DMA_CCR_MINC_Msk | DMA_CCR_DIR_Msk |
			     (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos);
	// Peripheral address
	DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;
	// Clear DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF3_Msk | DMA_IFCR_CGIF3_Msk;

	// Enable Timer4 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN_Msk;
	// Configure Timer
	// Disable timer
	TIM4->CR1 = 0;
	// No div, auto-reload, counts up, UEV enabled
	TIM4->CR1 = (0 << TIM_CR1_CKD_Pos) | TIM_CR1_ARPE_Msk | (0 << TIM_CR1_CMS_Pos);
	// Master mode disabled
	TIM4->CR2 = 0;
	// Slave mode disabled
	TIM4->SMCR = 0;
	// Prescaler by 1
	TIM4->PSC = 0;
	// Auto-reload counter from line frequency
	TIM4->ARR = clkTimer(4) / LPS - 1;
	TIM4->CNT = 0;
	// CC 1 and 2 disabled
	TIM4->CCMR1 = 0;
	// CC 3 and 4 disabled
	TIM4->CCMR2 = 0;
	// CC all disabled
	TIM4->CCER = 0;
	// Clear interrupt flags
	TIM4->SR = 0;
	// Update interrupt enabled
	TIM4->DIER = TIM_DIER_UIE_Msk;
	// Configure interrupt priority
	NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),
							NVIC_PRIORITY_MATRIX, 0));
	NVIC_EnableIRQ(TIM4_IRQn);

	// Initialise buffers and DMA
	matrix_buf_init(buffer);
	matrix_line();

	// Enable timer
	TIM4->CR1 |= TIM_CR1_CEN_Msk;

	printf(ESC_INIT "%lu\tmatrix: Initialisation done\n", systick_cnt());
}

INIT_HANDLER(&matrix_init);

static inline void matrix_buf_init(const uint8_t pbuf[][PANELS + 1])
{
	for (unsigned int line = 0; line < LINES; line++)
		for (unsigned int gs = 0; gs < GSCALE; gs++)
			memcpy(buf.buf[line * GSCALE + gs], pbuf[line * 2 + (gs == 0 ? 0 : 1)], PANELS + 1);
}

static inline void matrix_line()
{
	// Memory address
	DMA1_Channel3->CMAR = (uint32_t)(buf.buf[buf.ridx]);
	// Transfer size
	DMA1_Channel3->CNDTR = PANELS + 1;
	// Start DMA
	DMA1_Channel3->CCR |= DMA_CCR_EN_Msk;
}

void TIM4_IRQHandler()
{
	// Check DMA finished
	while (!(DMA1->ISR & DMA_ISR_TCIF3_Msk))
		dbgbkpt();
	// Check SPI finished
	while (SPI1->SR & SPI_SR_BSY_Msk)
		dbgbkpt();

	// Check line switching
	unsigned int linesw = buf.ridx % GSCALE;
	// Update latching registers
	if (linesw)
		// Disable OE
		GPIOB->BSRR = GPIO_BSRR_BS4_Msk;
	// Pulse ST
	GPIOA->BSRR = GPIO_BSRR_BS15_Msk;
	GPIOA->BSRR = GPIO_BSRR_BR15_Msk;
	if (linesw)
		// Enable OE
		GPIOB->BSRR = GPIO_BSRR_BR4_Msk;

	// Update line buffer index
	buf.ridx = (buf.ridx + 1) % (LINES * GSCALE);
	// Clear DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF3_Msk | DMA_IFCR_CGIF3_Msk;
	// Disable DMA
	DMA1_Channel3->CCR &= ~DMA_CCR_EN_Msk;
	// Start transferring a new line
	matrix_line();
	// Clear timer interrupt flags
	TIM4->SR = 0;
}
