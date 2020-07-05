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
#define LGSCALE	8
#define GSCALE	(1 << LGSCALE)
#define FPS	100

#define LPS	(FPS * LINES * GSCALE)
#define PPS	(LPS * (PANELS + 1) * 8)

//#define CHECK_OVERRUN
//#define PROFILING

#define DMA		DMA1
#define DMA_CHANNEL	DMA1_Channel3
#define DMA_CHANNEL_IRQ	DMA1_Channel3_IRQn

// Memory to peripheral, 8bit -> 8bit, very high priority
// Memory increment, peripheral not increment, circular disabled
#define DMA_CCR		((0b11 << DMA_CCR_PL_Pos) | DMA_CCR_MINC_Msk | DMA_CCR_DIR_Msk | \
			 (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos))

static struct {
	uint8_t buf[2][GSCALE][PANELS + 1];
	unsigned int rbuf, wbuf;
	unsigned int ridx;
	unsigned int lsw, bsw;
	unsigned int rline;
} buf = {0};

#if 0
static uint8_t fb[LINES][PANELS * 8] = {
	{0x01, 0x02, 0xff, 0x3f, 0x0f, 0x07, 0x03, 0x04,
	 0x01, 0x02, 0xff, 0x3f, 0x0f, 0x07, 0x03, 0x04,
	 0x01, 0x02, 0xff, 0x3f, 0x0f, 0x07, 0x03, 0x04,
	 0x01, 0x02, 0xff, 0x3f, 0x0f, 0x07, 0x03, 0x04,
	 0x01, 0x02, 0xff, 0x3f, 0x0f, 0x07, 0x03, 0x04},
	{0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00},
	{0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x3f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x3f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x3f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x3f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x3f},
	{0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
	 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
	 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
	 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
	 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f},
	{0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x0f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x0f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x0f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x0f,
	 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x0f},
	{0xff, 0x00, 0x00, 0x7f, 0x3f, 0x00, 0x00, 0x07,
	 0xff, 0x00, 0x00, 0x7f, 0x3f, 0x00, 0x00, 0x07,
	 0xff, 0x00, 0x00, 0x7f, 0x3f, 0x00, 0x00, 0x07,
	 0xff, 0x00, 0x00, 0x7f, 0x3f, 0x00, 0x00, 0x07,
	 0xff, 0x00, 0x00, 0x7f, 0x3f, 0x00, 0x00, 0x07},
	{0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
	 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00},
	{0x00, 0x00, 0xff, 0x7f, 0x3f, 0x1f, 0x00, 0x00,
	 0x00, 0x00, 0xff, 0x7f, 0x3f, 0x1f, 0x00, 0x00,
	 0x00, 0x00, 0xff, 0x7f, 0x3f, 0x1f, 0x00, 0x00,
	 0x00, 0x00, 0xff, 0x7f, 0x3f, 0x1f, 0x00, 0x00,
	 0x00, 0x00, 0xff, 0x7f, 0x3f, 0x1f, 0x00, 0x00},
};
#else
static uint8_t fb[LINES][PANELS * 8] = {
	{0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
	 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
	 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
	 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
	 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,},
	{0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
	 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,},
	{0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
	 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
	 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
	 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
	 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,},
	{0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
	 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
	 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
	 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
	 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,},
	{0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c,
	 0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c,
	 0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c,
	 0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c,
	 0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c,},
	{0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
	 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
	 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
	 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
	 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,},
	{0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc,
	 0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc,
	 0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc,
	 0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc,
	 0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc,},
	{0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc,
	 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc,
	 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc,
	 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc,
	 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc,},
};
#endif

static inline void matrix_buf_init();
static inline void matrix_line();
static inline void matrix_line_calc();

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
	// Configure DMA Channel 3 for SPI1_TX
	// Disable stream
	DMA_CHANNEL->CCR = 0;
	// Peripheral address
	DMA_CHANNEL->CPAR = (uint32_t)&SPI1->DR;
	// Clear DMA complete flag
	DMA->IFCR = DMA_IFCR_CTCIF3_Msk | DMA_IFCR_CGIF3_Msk;

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
	// Configure interrupts
	uint32_t pg = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_MATRIX, 0));
	NVIC_EnableIRQ(TIM4_IRQn);
	// Use DMA interrupt as calculation thread
	NVIC_SetPriority(DMA_CHANNEL_IRQ, NVIC_EncodePriority(pg, NVIC_PRIORITY_MATRIX_CALC, 0));
	NVIC_EnableIRQ(DMA_CHANNEL_IRQ);

	// Initialise buffers and DMA
	matrix_buf_init();
	matrix_line();

	// Enable timer
	TIM4->CR1 |= TIM_CR1_CEN_Msk;

	printf(ESC_INIT "%lu\tmatrix: Initialisation done\n", systick_cnt());
}

INIT_HANDLER(&matrix_init);

static inline void matrix_buf_init()
{
	buf.rbuf = 1;
	buf.wbuf = 0;
	matrix_line_calc();
}

static inline void matrix_line_calc()
{
	unsigned int line = buf.rline;
	// Update line counter
	buf.rline = (buf.rline + 1) % LINES;
	// Interleaaved, line order 0, 4, 2, 6, 1, 5, 3, 7
	line = (line & ~5) | ((line & 1) << 2) | ((line & 4) >> 2);

	// Generate grayscale buffer
	unsigned int wbuf = !!buf.wbuf;
	uint8_t *pfbs = &fb[line][0];
	uint8_t lmask = ~(1 << line);
	for (unsigned int pnl = 0; pnl < PANELS; pnl++) {
		uint8_t *pfb = pfbs + pnl * 8;
		uint8_t v = 0x00;
		uint8_t step = 0;
		uint8_t next;
		uint8_t *pbuf = &buf.buf[wbuf][0][pnl];
		do {
			next = GSCALE - 1;
			// Find next grayscale change
			for (unsigned int i = 0; i < 8; i++) {
				uint8_t vfb = pfb[i];
				if (vfb > step && vfb < next)
					next = vfb;
			}
			// Update grayscale value
			for (unsigned int i = 0; i < 8; i++)
				if (pfb[i] == step)
					v |= 0x80 >> i;
			// Apply current grayscale value
			while (step != next) {
				*pbuf = v;
				pbuf += PANELS + 1;
				step++;
			}
		} while (next != GSCALE - 1);
		buf.buf[wbuf][GSCALE - 1][pnl] = 0xff;
	}

	// Update line driver
	uint8_t *pbuf = &buf.buf[wbuf][0][PANELS];
	for (unsigned int gs = 0; gs < GSCALE; gs++) {
		*pbuf = lmask;
		pbuf += PANELS + 1;
	}

	// Done, switch active buffer
	buf.wbuf = !wbuf;
}

#ifdef PROFILING
static volatile unsigned int irq = 0;
#endif

void DMA1_Channel3_IRQHandler()
{
	// Line buffer calculation thread
#ifdef PROFILING
	unsigned int s = irq;
#endif
	matrix_line_calc();
#ifdef PROFILING
	unsigned int e = irq;

	static unsigned int i = 8;
	if (i) {
		printf(ESC_DEBUG "matrix: irq %u -> %u\n", s, e);
		i--;
	}
#endif
}

static inline void matrix_line()
{
	if (buf.bsw)
		buf.rbuf = !buf.wbuf;
	void *next = &buf.buf[buf.rbuf][buf.ridx];
	// Disable DMA
	DMA_CHANNEL->CCR = DMA_CCR;
	// Transfer size
	DMA_CHANNEL->CNDTR = PANELS + 1;
	// Memory address
	DMA_CHANNEL->CMAR = (uint32_t)(next);
	// Start DMA
	DMA_CHANNEL->CCR = DMA_CCR | DMA_CCR_EN_Msk;
#ifdef CHECK_OVERRUN
	// Clear DMA complete flag
	DMA->IFCR = DMA_IFCR_CTCIF3_Msk | DMA_IFCR_CGIF3_Msk;
#endif

	// Line switching after DMA complete
	buf.lsw = buf.ridx == 0;
	// Update line buffer index
	buf.ridx = (buf.ridx + 1) % GSCALE;
	// Trigger buffer update
	if (buf.bsw)
		NVIC_SetPendingIRQ(DMA_CHANNEL_IRQ);
	// Buffer switching after DMA complete
	buf.bsw = buf.ridx == 0;
}

void TIM4_IRQHandler()
{
	// Update latching registers
	unsigned int lsw = !!buf.lsw;
	// Disable OE
	if (lsw)
		GPIOB->BSRR = GPIO_BSRR_BS4_Msk;
	// Pulse ST
	GPIOA->BSRR = GPIO_BSRR_BS15_Msk;
	GPIOA->BSRR = GPIO_BSRR_BR15_Msk;
	// Enable OE
	if (lsw)
		GPIOB->BSRR = GPIO_BSRR_BR4_Msk;

#ifdef CHECK_OVERRUN
	// Check DMA finished
	while (!(DMA->ISR & DMA_ISR_TCIF3_Msk))
		dbgbkpt();
	// Check SPI finished
	while (SPI1->SR & SPI_SR_BSY_Msk)
		dbgbkpt();
#endif

	// Start transferring a new line
	matrix_line();
	// Clear timer interrupt flags
	TIM4->SR = 0;

#ifdef PROFILING
	irq++;
#endif
}
