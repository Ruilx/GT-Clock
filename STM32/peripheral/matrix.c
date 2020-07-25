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
#define FPS	120

// Last GSCALE is unused
#define LPS	(FPS * LINES * (GSCALE - 1))
// Last PANEL byte is line driver
#define PPS	(LPS * (PANELS + 1) * 8)

#if DEBUG > 5
#define CHECK_OVERRUN
#endif
//#define PROFILING
//#define GSUPD_FAST

#define DMA		DMA1
#define DMA_CHANNEL	DMA1_Channel3
#define DMA_CHANNEL_IRQ	DMA1_Channel3_IRQn

// Memory to peripheral, 8bit -> 8bit, very high priority
// Memory increment, peripheral not increment, circular disabled
#define DMA_CCR		((0b11 << DMA_CCR_PL_Pos) | DMA_CCR_MINC_Msk | DMA_CCR_DIR_Msk | \
			 (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos))

#define BUF_SIZE	(PANELS + 1)

static struct {
	// Line processing buffer
	uint8_t buf[2][GSCALE - 1][BUF_SIZE];
#ifdef GSUPD_FAST
	uint8_t gsupd[2][GSCALE - 1];
#else
	uint8_t gsupd[2][GSCALE / 8];
#endif

	uint8_t rbuf;		// Active read buffer
	uint8_t ridx;		// Read index
	uint8_t lsw;		// Line switching
	uint8_t bsw;		// Buffer switching
	uint8_t gssw;		// Greyscaling update

	uint8_t wbuf;		// Pending write buffer
	uint8_t wline;		// Pending write line

	// Frame buffer
	uint8_t fb[2][LINES][PANELS * 8];

	volatile uint8_t rfb;	// Active frame buffer
	uint8_t wfb;		// Standby frame buffer
	unsigned int refcnt;	// Frame refresh counter
} data = {0};

static inline void matrix_buf_init();
static inline void matrix_line();
static inline void matrix_line_calc();

static void matrix_init()
{
	// Configure GPIOs
	// PB3  SH SPI1: Alternate function output push-pull, 50MHz
	// PB4  OE:      Output push-pull, 50MHz
	// PB5  DS SPI1: Alternate function output push-pull, 50MHz
	// PA15 ST:      Output push-pull, 50MHz
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN_Msk | RCC_APB2ENR_IOPBEN_Msk | RCC_APB2ENR_AFIOEN_Msk;
	GPIOB->CRL = (GPIOB->CRL & ~(GPIO_CRL_CNF3_Msk | GPIO_CRL_MODE3_Msk |
				     GPIO_CRL_CNF4_Msk | GPIO_CRL_MODE4_Msk |
				     GPIO_CRL_CNF5_Msk | GPIO_CRL_MODE5_Msk)) |
		     ((0b10 << GPIO_CRL_CNF3_Pos) | (0b11 << GPIO_CRL_MODE3_Pos)) |
		     ((0b00 << GPIO_CRL_CNF4_Pos) | (0b11 << GPIO_CRL_MODE4_Pos)) |
		     ((0b10 << GPIO_CRL_CNF5_Pos) | (0b11 << GPIO_CRL_MODE5_Pos));
	GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF15_Msk | GPIO_CRH_MODE15_Msk)) |
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
	// No div, auto-reload, counts up, update event enabled
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

	// Initialise buffers and start DMA
	matrix_buf_init();
	matrix_line();

	// Enable timer
	TIM4->CR1 |= TIM_CR1_CEN_Msk;

#if DEBUG > 5
	printf(ESC_INIT "%lu\tmatrix: Init done\n", systick_cnt());
#endif
}

INIT_HANDLER() = &matrix_init;

static inline void matrix_buf_init()
{
	data.rbuf = 0;
	data.wbuf = 1;
	data.wfb = 1;
	data.rfb = 0;
	// Disable all line drivers
	for (unsigned int gs = 0; gs < GSCALE - 1; gs++)
		data.buf[data.rbuf][gs][PANELS] = 0xff;

#if 0
	// Test pattern
	for (unsigned int line = 0; line < LINES; line++)
		for (unsigned int pnl = 0; pnl < PANELS && pnl < 4; pnl++)
			for (unsigned int i = 0; i < 8; i++)
				data.fb[0][line][pnl * 8 + i] = line * 4 * 8 + pnl * 8 + i;
	for (unsigned int line = 0; line < LINES; line++)
		for (unsigned int pnl = 4; pnl < PANELS; pnl++)
			for (unsigned int i = 0; i < 8; i++)
				data.fb[0][line][pnl * 8 + i] = (line * 8 + i) * 4;
#if 0
	for (unsigned int line = 0; line < LINES; line++)
		for (unsigned int pnl = 3; pnl < PANELS; pnl += 2)
			for (unsigned int i = 0; i < 16; i++)
				data.fb[0][line][pnl * 8 + i] = (line * 8 + i) * 2;
#endif
	for (unsigned int line = 0; line < LINES; line++)
		for (unsigned int pix = 0; pix < PANELS * 8; pix++)
			if ((pix % 2 == 1) ^ (line % 2 == 0))
				data.fb[0][line][pix] = 0;
#endif
}

static inline void matrix_line_calc()
{
	unsigned int line = data.wline;
	// Update line counter
	data.wline = (line + 1) % LINES;
	// Interleaaved, line order 0, 4, 2, 6, 1, 5, 3, 7
	line = (line & ~5) | ((line & 1) << 2) | ((line & 4) >> 2);

	// Generate grayscale buffer
	unsigned int wbuf = !!data.wbuf;
	uint8_t *pfbs = &data.fb[data.rfb][line][0];
	uint8_t lmask = ~(1 << line);
	memset(data.gsupd[wbuf], 0, sizeof(data.gsupd[0]));
	for (unsigned int pnl = 0; pnl < PANELS; pnl++) {
		uint8_t *pfb = pfbs + pnl * 8;
		uint8_t v = 0x00;
		uint8_t step = 0;
		uint8_t next;
		uint8_t *pbuf = &data.buf[wbuf][0][pnl];
		do {
			next = GSCALE - 1;
			// Find next grayscale change
			for (unsigned int i = 0; i < 8; i++)
				if (pfb[i] > step && pfb[i] < next)
					next = pfb[i];
			// Update grayscale value
			for (unsigned int i = 0; i < 8; i++)
				if (pfb[i] == step)
					v |= 0x80 >> i;
			// Apply current grayscale value
#ifdef GSUPD_FAST
			data.gsupd[wbuf][step] = 1;
#else
			data.gsupd[wbuf][step / 8] |= 1 << (step % 8);
#endif
			while (step != next) {
				step++;
				*pbuf = v;
				pbuf += BUF_SIZE;
			}
		} while (next != GSCALE - 1);
	}

	// Update line driver
	for (unsigned int gs = 0; gs < GSCALE - 1; gs++)
		data.buf[wbuf][gs][PANELS] = lmask;

	// Done, switch active buffer
	data.wbuf = !wbuf;
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
	if (data.wline == 0) {
		data.rfb = !data.wfb;
		data.refcnt++;
	}
	matrix_line_calc();
#ifdef PROFILING
	unsigned int e = irq;

	static unsigned int i = 8;
	if (i) {
		printf(ESC_DEBUG "%lu\tmatrix: irq %u -> %u\n", systick_cnt(), s, e);
		i--;
	}
#endif
}

unsigned int matrix_refresh_cnt()
{
	return data.refcnt;
}

static inline void matrix_line()
{
	if (data.bsw)
		data.rbuf = !data.wbuf;
#ifdef GSUPD_FAST
	data.gssw = data.gsupd[data.rbuf][data.ridx];
#else
	data.gssw = !!(data.gsupd[data.rbuf][data.ridx / 8] & (1 << (data.ridx % 8)));
#endif
	if (data.gssw) {
		// Disable DMA
		DMA_CHANNEL->CCR = DMA_CCR;
		// Transfer size
		DMA_CHANNEL->CNDTR = PANELS + 1;
		// Memory address
		DMA_CHANNEL->CMAR = (uint32_t)&data.buf[data.rbuf][data.ridx];
		// Start DMA
		DMA_CHANNEL->CCR = DMA_CCR | DMA_CCR_EN_Msk;
#ifdef CHECK_OVERRUN
		// Clear DMA complete flag
		DMA->IFCR = DMA_IFCR_CTCIF3_Msk | DMA_IFCR_CGIF3_Msk;
#endif
	}

	// Line switching after DMA complete
	data.lsw = data.ridx == 0;
	// Update line buffer index
	data.ridx++;
	if (data.ridx == GSCALE - 1)
		data.ridx = 0;
	//buf.ridx = (buf.ridx + 1) % GSCALE;
	// Trigger buffer update
	if (data.bsw)
		NVIC_SetPendingIRQ(DMA_CHANNEL_IRQ);
	// Buffer switching after DMA complete
	data.bsw = data.ridx == 0;
}

void TIM4_IRQHandler()
{
	// Update latching registers
	if (data.gssw) {
		// Disable OE
		if (data.lsw)
			GPIOB->BSRR = GPIO_BSRR_BS4_Msk;
		// Pulse ST
		GPIOA->BSRR = GPIO_BSRR_BS15_Msk;
		GPIOA->BSRR = GPIO_BSRR_BR15_Msk;
		// Enable OE
		if (data.lsw)
			GPIOB->BSRR = GPIO_BSRR_BR4_Msk;
	}

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
	NVIC_ClearPendingIRQ(TIM4_IRQn);

#ifdef PROFILING
	irq++;
#endif
}

void *matrix_fb(unsigned int active, unsigned int *w, unsigned int *h)
{
	*w = PANELS * 8;
	*h = LINES;
	return &data.fb[active ^ data.wfb][0][0];
}

unsigned int matrix_fb_ready()
{
	return data.rfb != data.wfb;
}

void matrix_fb_swap()
{
	data.wfb = !data.wfb;
}

void matrix_fb_copy()
{
	memcpy(&data.fb[data.wfb][0][0], &data.fb[!data.wfb][0][0], LINES * PANELS * 8);
}
