#include <stdio.h>
#include <string.h>
#include <device.h>
#include <debug.h>
#include <system/systick.h>
#include <system/clocks.h>
#include <system/irq.h>
#include "i2c_slave.h"

#define I2C_ADDR	0x1c

#define DEBUG_PRINT	6
#define DEBUG_EVLOG	6
#define DEBUG_CHECK	5
#define DEBUG_IRQ	6

#if DEBUG >= DEBUG_CHECK
#define BUF_SIZE	4
#else
#define BUF_SIZE	16
#endif

LIST(i2c_slave_reg, i2c_slave_reg_handler_t);

typedef enum {Idle = 0, WaitTx, ActiveTx, ActiveRx} state_t;
typedef enum {BufInvalid = 0,
	      BufTx = 4, BufTxSegment = 5, BufTxComplete = 6,
	      BufRx = 8, BufRxSegment = 9, BufRxComplete = 10} buf_t;

typedef enum {RegIdle = 0, RegAddr, RegWrite, RegRead} reg_t;
typedef enum {RegBufInvalid = 0, RegBufSegment, RegBufComplete} reg_buf_t;

static struct {
	volatile state_t state;
	struct {
		volatile unsigned int dmaidx;
		volatile unsigned int intwait;
		unsigned int procidx;
		struct {
			buf_t state;
			unsigned int size;
			uint8_t data[BUF_SIZE];
		} buf[2];
	} buf;

	struct {
		struct {
			reg_buf_t state;
			unsigned int size;
			unsigned int pos;
			uint8_t *p;
		} buf;

		reg_t state;
		uint8_t id;
		unsigned int segment;
	} reg;
} data = {0};

#if DEBUG >= DEBUG_EVLOG
#define EVLOG_SIZE	16
struct {
	unsigned int idx;
	struct {
		uint16_t sr1;
		state_t state;
		reg_t regstate;
		unsigned int dmaidx;
		buf_t dmastate[2];
	} log[EVLOG_SIZE];
} evlog = {0};
#endif

static void i2c_slave_dma_rx();

static void i2c_slave_init()
{
	// Configure GPIOs
	// PB10 SCL I2C2: Alternate function open-drain, 10MHz
	// PB11 SDA I2C2: Alternate function open-drain, 10MHz
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN_Msk | RCC_APB2ENR_AFIOEN_Msk;
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF10_Msk | GPIO_CRH_MODE10_Msk |
				     GPIO_CRH_CNF11_Msk | GPIO_CRH_MODE11_Msk)) |
		     ((0b11 << GPIO_CRH_CNF10_Pos) | (0b01 << GPIO_CRH_MODE10_Pos)) |
		     ((0b11 << GPIO_CRH_CNF11_Pos) | (0b01 << GPIO_CRH_MODE11_Pos));

	// Enable I2C2 clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN_Msk;
	// Disable and configure I2C peripheral
	// Clock stretching enabled, general call disabled, I2C mode
	I2C2->CR1 = 0;
	// DMA enabled, buffer interrupts disabled, event interrupts enabled
	// Set APB clock frequency
	I2C2->CR2 = I2C_CR2_DMAEN_Msk | I2C_CR2_ITEVTEN_Msk | I2C_CR2_ITERREN_Msk |
		    ((clkAPB1() / 1000000) << I2C_CR2_FREQ_Pos);
	// Configure I2C address, 7-bit mode
	I2C2->OAR1 = (I2C_ADDR << I2C_OAR1_ADD1_Pos);
	// Dual address disabled
	I2C2->OAR2 = 0;
	// Clear flags
	I2C2->SR1 = 0;
	// Clock control, 400kHz fast mode
	I2C2->CCR = I2C_CCR_FS_Msk | I2C_CCR_DUTY_Msk | (4 << I2C_CCR_CCR_Pos);

	// Enable DMA clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN_Msk;

	// Configure DMA Channel 4 for I2C2_TX
	// Disable stream
	DMA1_Channel4->CCR = 0;
	// Peripheral address
	DMA1_Channel4->CPAR = (uint32_t)&I2C2->DR;
	// Clear DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF4_Msk | DMA_IFCR_CGIF4_Msk;
	// Memory to peripheral, 8bit -> 8bit, low priority
	// Memory increment, peripheral not increment, circular disabled
	// Transfer complete interrupt enable
	DMA1_Channel4->CCR = (0b00 << DMA_CCR_PL_Pos) | DMA_CCR_MINC_Msk | DMA_CCR_DIR_Msk | \
			     (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos) |
			     DMA_CCR_TCIE_Msk;

	// Configure DMA Channel 5 for I2C2_RX
	// Disable stream
	DMA1_Channel5->CCR = 0;
	// Peripheral address
	DMA1_Channel5->CPAR = (uint32_t)&I2C2->DR;
	// Clear DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF5_Msk | DMA_IFCR_CGIF5_Msk;
	// Peripheral to memory, 8bit -> 8bit, low priority
	// Memory increment, peripheral not increment, circular disabled
	// Transfer complete interrupt enable
	DMA1_Channel5->CCR = (0b00 << DMA_CCR_PL_Pos) | DMA_CCR_MINC_Msk | /*DMA_CCR_DIR_Msk |*/ \
			     (0b00 << DMA_CCR_MSIZE_Pos) | (0b00 << DMA_CCR_PSIZE_Pos) |
			     DMA_CCR_TCIE_Msk;

	// Configure interrupts
	uint32_t pg = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(I2C2_EV_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_I2C, 0));
	NVIC_EnableIRQ(I2C2_EV_IRQn);
	NVIC_SetPriority(I2C2_ER_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_I2C, 0));
	NVIC_EnableIRQ(I2C2_ER_IRQn);
	NVIC_SetPriority(DMA1_Channel4_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_I2C_DMA, 0));
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	NVIC_SetPriority(DMA1_Channel5_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_I2C_DMA, 0));
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);
	// Enable I2C peripheral
	// ACK enable
	I2C2->CR1 = I2C_CR1_PE_Msk | I2C_CR1_ACK_Msk;

#if DEBUG >= DEBUG_PRINT
	printf(ESC_INIT "%lu\ti2c: Init done\n", systick_cnt());
#endif
}

INIT_HANDLER() = &i2c_slave_init;

static inline void i2c_slave_irq_rx_wait()
{
	// Wait for RX buffer
	// Disable interrupts for now
	I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
	data.buf.intwait = 1;
}

static void i2c_slave_dma_rx()
{
#if DEBUG >= DEBUG_CHECK
	if (data.buf.buf[data.buf.dmaidx].state != BufInvalid)
		dbgbkpt();
#endif
	// Disable DMAs
	DMA1_Channel4->CCR &= ~DMA_CCR_EN_Msk;
	DMA1_Channel5->CCR &= ~DMA_CCR_EN_Msk;
	// Transfer size
	DMA1_Channel5->CNDTR = BUF_SIZE;
	// Memory address
	DMA1_Channel5->CMAR = (uint32_t)data.buf.buf[data.buf.dmaidx].data;
	// Set buffer states
	data.buf.buf[data.buf.dmaidx].state = BufRx;
	data.buf.buf[data.buf.dmaidx].size = 0;
	// Start RX DMA
	DMA1_Channel5->CCR |= DMA_CCR_EN_Msk;
}

static inline void i2c_slave_dma_irq_rx_segment()
{
#if DEBUG >= DEBUG_CHECK
	if (data.buf.buf[data.buf.dmaidx].state != BufRx)
		dbgbkpt();
#endif
	// Set buffer states
	data.buf.buf[data.buf.dmaidx].size = BUF_SIZE - DMA1_Channel5->CNDTR;
	data.buf.buf[data.buf.dmaidx].state = BufRxSegment;
	// Update double buffering
	data.buf.dmaidx = !data.buf.dmaidx;
	// Disable RX DMA
	//DMA1_Channel4->CCR &= ~DMA_CCR_EN_Msk;
	DMA1_Channel5->CCR &= ~DMA_CCR_EN_Msk;
	// Clear RX DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF5_Msk | DMA_IFCR_CGIF5_Msk;
	// Don't restart DMA, wait for BTF interrupt to avoid I2C bug
}

static void i2c_slave_irq_rx_dma_done()
{
	// Set buffer states
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		data.buf.buf[data.buf.dmaidx].size = 0;
		break;
	case BufRx:
		data.buf.buf[data.buf.dmaidx].size = BUF_SIZE - DMA1_Channel5->CNDTR;
		break;
	default:
#if DEBUG >= DEBUG_CHECK
		dbgbkpt();
#endif
		break;
	}

	data.buf.buf[data.buf.dmaidx].state = BufRxComplete;
	// Update double buffering
	data.buf.dmaidx = !data.buf.dmaidx;
	// Disable RX DMA
	DMA1_Channel5->CCR &= ~DMA_CCR_EN_Msk;
}

static void i2c_slave_irq_tx_dma_done()
{
	// Set buffer states
	data.buf.buf[data.buf.dmaidx].state = BufInvalid;
	// Update double buffering
	data.buf.dmaidx = !data.buf.dmaidx;
	// Disable TX DMA
	DMA1_Channel4->CCR &= ~DMA_CCR_EN_Msk;
}

static void i2c_slave_irq_rx_next()
{
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		i2c_slave_dma_rx();
		break;
	default:
		i2c_slave_irq_rx_wait();
	}
}

static void i2c_slave_irq_done()
{
#if DEBUG >= DEBUG_PRINT
	printf(ESC_DISABLE "%lu\ti2c irq: stop\n", systick_cnt());
#endif
	// We may have 1 byte left in I2C receive buffer
	if (I2C2->SR1 & I2C_SR1_RXNE_Msk) {
		switch (data.buf.buf[data.buf.dmaidx].state) {
		case BufInvalid:
			// Read that last 1 byte
			// Set buffer states
			data.buf.buf[data.buf.dmaidx].data[0] = I2C2->DR;
			data.buf.buf[data.buf.dmaidx].size = 1;
			data.buf.buf[data.buf.dmaidx].state = BufRxComplete;
			// Update double buffering
			data.buf.dmaidx = !data.buf.dmaidx;
			// Re-enable ACK and clear stop flag
			I2C2->CR1 |= I2C_CR1_ACK_Msk;
			data.state = Idle;
			break;
#if DEBUG >= DEBUG_CHECK
		case BufRx:
			// DMA should be finished and disabled
			dbgbkpt();
#endif
			// fall through
		default:
			// Or, wait for buffer
			i2c_slave_irq_rx_wait();
		}
		return;
	}

	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
	case BufRx:
		i2c_slave_irq_rx_dma_done();
		// Re-enable ACK and clear stop flag
		I2C2->CR1 |= I2C_CR1_ACK_Msk;
		data.state = Idle;
		break;
#if DEBUG >= DEBUG_CHECK
	case BufTxComplete:
	case BufTxSegment:
	case BufTx:
		dbgbkpt();
		break;
#endif
	default:
		// Wait for RX buffer for end indication
		i2c_slave_irq_rx_wait();
		break;
	}
}


static void i2c_slave_irq_tx_done()
{
#if DEBUG >= DEBUG_PRINT
	printf(ESC_DISABLE "%lu\ti2c irq: stop\n", systick_cnt());
#endif
#if DEBUG >= DEBUG_CHECK
	// We may have 1 byte left in I2C receive buffer
	if (I2C2->SR1 & I2C_SR1_RXNE_Msk)
		dbgbkpt();
#endif

	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufTx:
		// Wait for buffer to be filled, then clean it
		data.buf.intwait = 1;
		// Disable interrupts
		I2C2->CR2 &= ~(I2C_CR2_ITEVTEN_Msk | I2C_CR2_ITERREN_Msk);
		return;
	case BufTxComplete:
	case BufTxSegment:
		i2c_slave_irq_tx_dma_done();
		// fall through
	case BufInvalid:
		// Clear NAK flag
		I2C2->SR1 = ~I2C_SR1_AF_Msk;
		data.state = Idle;
		break;
	default:
#if DEBUG >= DEBUG_CHECK
		dbgbkpt();
#endif
		break;
	}
	// Reenable interrupts
	I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk;
}

static void i2c_slave_irq_tx_wait()
{
	// Wait for TX buffer
	if (data.buf.buf[data.buf.dmaidx].state == BufInvalid)
		data.buf.buf[data.buf.dmaidx].state = BufTx;
	data.buf.intwait = 1;
	// Disable interrupts for now
	I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
}

static void i2c_slave_dma_irq_tx_segment()
{
#if DEBUG >= DEBUG_CHECK
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufTxSegment:
	case BufTxComplete:
		break;
	default:
		dbgbkpt();
	}
#endif

	i2c_slave_irq_tx_dma_done();
	i2c_slave_irq_tx_wait();
	// Clear TX DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF4_Msk | DMA_IFCR_CGIF4_Msk;
}

static void i2c_slave_dma_tx()
{
#if DEBUG >= DEBUG_CHECK
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufTxSegment:
	case BufTxComplete:
		break;
	default:
		dbgbkpt();
	}
#endif

	// Disable DMAs
	DMA1_Channel4->CCR &= ~DMA_CCR_EN_Msk;
	DMA1_Channel5->CCR &= ~DMA_CCR_EN_Msk;
	// Set circular mode and disable interrupt if TX complete
	uint32_t ccr = data.buf.buf[data.buf.dmaidx].state == BufTxComplete ?
			       DMA_CCR_CIRC_Msk : DMA_CCR_TCIE_Msk;
	DMA1_Channel4->CCR = (DMA1_Channel4->CCR & ~(DMA_CCR_CIRC_Msk | DMA_CCR_TCIE_Msk)) | ccr;
	// Transfer size
	DMA1_Channel4->CNDTR = data.buf.buf[data.buf.dmaidx].size;
	// Memory address
	DMA1_Channel4->CMAR = (uint32_t)data.buf.buf[data.buf.dmaidx].data;
	// Start TX DMA
	DMA1_Channel4->CCR |= DMA_CCR_EN_Msk;
}

static void i2c_slave_irq_tx_start()
{
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufTxSegment:
	case BufTxComplete:
		break;
	default:
		i2c_slave_irq_tx_wait();
		return;
	}

	data.state = ActiveTx;
	// TX DMA start
	i2c_slave_dma_tx();
}

static void i2c_slave_irq_addr()
{
	// Addressed, switch to transmitter or receiver mode
	uint16_t sr2 = I2C2->SR2;
#if DEBUG >= DEBUG_PRINT
	printf(ESC_ENABLE "%lu\ti2c irq: addr 0x%04x\n", systick_cnt(), sr2);
#endif
	if (sr2 & I2C_SR2_TRA_Msk) {
		i2c_slave_irq_tx_wait();
		data.state = WaitTx;
	} else {
		// Wait until BTF interrupt for data transfer to avoid I2C bug
		data.state = ActiveRx;
	}
}

void DMA1_Channel4_IRQHandler()
{
#if DEBUG >= DEBUG_CHECK
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BS14_Msk;
	GPIOB->BSRR = GPIO_BSRR_BR14_Msk;
	GPIOB->BSRR = GPIO_BSRR_BS14_Msk;
#endif
	i2c_slave_dma_irq_tx_segment();
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BR14_Msk;
	GPIOB->BSRR = GPIO_BSRR_BS14_Msk;
	GPIOB->BSRR = GPIO_BSRR_BR14_Msk;
#endif
}

void DMA1_Channel5_IRQHandler()
{
#if DEBUG >= DEBUG_CHECK
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BS14_Msk;
#endif
	i2c_slave_dma_irq_rx_segment();
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BR14_Msk;
#endif
}

void I2C2_EV_IRQHandler()
{
	uint16_t sr1 = I2C2->SR1;
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BS12_Msk;
#endif
	if (!sr1) {
#if DEBUG >= DEBUG_IRQ
		GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
#endif
		return;
	}

#if DEBUG >= DEBUG_CHECK
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif
#if DEBUG >= DEBUG_EVLOG
	evlog.log[evlog.idx].sr1 = sr1;
	evlog.log[evlog.idx].state = data.state;
	evlog.log[evlog.idx].dmaidx = data.buf.dmaidx;
	evlog.log[evlog.idx].dmastate[0] = data.buf.buf[0].state;
	evlog.log[evlog.idx].dmastate[1] = data.buf.buf[1].state;
	evlog.log[evlog.idx].regstate = data.reg.state;
#endif
#if DEBUG >= DEBUG_PRINT
	printf(ESC_DEBUG "%lu\ti2c irq: SR1 0x%04x\n", systick_cnt(), sr1);
	//flushCache();
#endif

	// For slave mode, possible event interrupts are:
	// ADDR		Address matched
	// STOPF	Stop condition
	// BTF only	???
	// TXE		Transmit buffer empty
	// RXNE		Receive buffer not empty

	switch (data.state) {
	case Idle:
		if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_irq_addr();
		else if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_irq_done();
#if DEBUG >= DEBUG_CHECK
		else
			dbgbkpt();
#endif
		break;
	case WaitTx:
		i2c_slave_irq_tx_start();
		break;
	case ActiveTx:
		if (sr1 & I2C_SR1_BTF_Msk) {
			// Disable buffer interrupt again
			I2C2->CR2 &= ~I2C_CR2_ITBUFEN_Msk;
			i2c_slave_irq_tx_start();
		} else if (sr1 & I2C_SR1_AF_Msk) {
			// No acknowledgement
			i2c_slave_irq_tx_done();
		}
#if 0
		if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
#endif
#if DEBUG >= DEBUG_CHECK
		else
			dbgbkpt();
#endif
		break;
	case ActiveRx:
		if (sr1 & I2C_SR1_STOPF_Msk) {
#if DEBUG >= DEBUG_CHECK
			// Stop is only generated after the last data byte is transferred
			if(sr1 & I2C_SR1_BTF_Msk)
				dbgbkpt();
#endif
			i2c_slave_irq_done();
		} else if(sr1 & I2C_SR1_BTF_Msk) {
			i2c_slave_irq_rx_next();
		} else if (sr1 & I2C_SR1_ADDR_Msk) {
#if DEBUG >= DEBUG_CHECK
			// Stop is only generated after the last data byte is transferred
			if(sr1 & I2C_SR1_BTF_Msk)
				dbgbkpt();
#endif
			i2c_slave_irq_done();
			i2c_slave_irq_addr();
		}
#if DEBUG >= DEBUG_CHECK
		else
			dbgbkpt();
#endif
		break;
	}

#if DEBUG >= DEBUG_EVLOG
	evlog.idx = evlog.idx == EVLOG_SIZE - 1 ? 0 : evlog.idx + 1;
#endif
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
#endif
}

void I2C2_ER_IRQHandler()
{
	uint16_t sr1 = I2C2->SR1;
#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BS12_Msk;
	GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
	GPIOB->BSRR = GPIO_BSRR_BS12_Msk;
#endif

#if DEBUG >= DEBUG_PRINT
	printf(ESC_DEBUG "%lu\ti2c err irq: SR1 0x%04x\n", systick_cnt(), sr1);
	//flushCache();
#endif

	if (sr1 & I2C_SR1_AF_Msk) {
		// No acknowledgement
		i2c_slave_irq_tx_done();
	} else if (sr1 & I2C_SR1_BERR_Msk) {
		// Bus error
		I2C2->SR1 = ~I2C_SR1_BERR_Msk;
#if DEBUG >= DEBUG_CHECK
	} else if (sr1 & (I2C_SR1_ARLO_Msk | I2C_SR1_OVR_Msk | I2C_SR1_TIMEOUT_Msk |
			  I2C_SR1_PECERR_Msk | I2C_SR1_SMBALERT_Msk)) {
		dbgbkpt();
#endif
	}

#if DEBUG >= DEBUG_IRQ
	GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
	GPIOB->BSRR = GPIO_BSRR_BS12_Msk;
	GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
#endif
}

static void *i2c_slave_reg_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	LIST_ITERATE(i2c_slave_reg, i2c_slave_reg_handler_t, phdr) {
		void *p = phdr->data(write, id, segment, size);
		if (p)
			return p;
	}

	*segment = 0;
	*size = 0;
	return 0;
}

static void i2c_slave_reg_rx_complete(unsigned int id, unsigned int segment, unsigned int size, void *p)
{
	LIST_ITERATE(i2c_slave_reg, i2c_slave_reg_handler_t, phdr)
		if (phdr->write_complete)
			phdr->write_complete(id, segment, size, p);

#if DEBUG >= DEBUG_PRINT
	printf(ESC_WRITE "%lu\ti2c reg: Write complete 0x%02x, segment %u: %u bytes\n",
	       systick_cnt(), data.reg.id, data.reg.segment, data.reg.buf.size);
#endif
}

static void i2c_slave_reg_rx_buf()
{
	switch (data.reg.state) {
	case RegIdle:
	case RegRead:
		// Register idle, wait for ID
#if DEBUG >= DEBUG_CHECK
		if (data.reg.buf.state != RegBufInvalid)
			dbgbkpt();
#endif
		data.reg.segment = 0;
		data.reg.state = RegAddr;
		data.reg.buf.p = &data.reg.id;
		data.reg.buf.size = 1;
		data.reg.buf.pos = 0;
		data.reg.buf.state = RegBufSegment;
		break;
	case RegAddr:
		// Register ID received
#if DEBUG >= DEBUG_CHECK
		if (data.reg.buf.state != RegBufSegment || data.reg.buf.pos != 1)
			dbgbkpt();
#endif
		data.reg.segment = 0;
		data.reg.state = RegWrite;
		data.reg.buf.p = i2c_slave_reg_data(1, data.reg.id, &data.reg.segment, &data.reg.buf.size);
		data.reg.buf.pos = 0;
		data.reg.buf.state = data.reg.segment ? RegBufSegment : RegBufComplete;
		break;
	case RegWrite:
		// Register write segment complete
#if DEBUG >= DEBUG_CHECK
		if (data.reg.buf.state != RegBufSegment || data.reg.buf.pos != data.reg.buf.size)
			dbgbkpt();
#endif
		data.reg.buf.p = i2c_slave_reg_data(1, data.reg.id, &data.reg.segment, &data.reg.buf.size);
		data.reg.buf.pos = 0;
		data.reg.buf.state = data.reg.segment ? RegBufSegment : RegBufComplete;
		break;
	default:
#if DEBUG >= DEBUG_CHECK
		dbgbkpt();
#endif
		break;
	}

#if DEBUG >= DEBUG_PRINT
	printf(ESC_WRITE "%lu\ti2c reg: Write buffer segment %u: %u bytes\n",
	       systick_cnt(), data.reg.segment, data.reg.buf.size);
#endif
}

static inline void i2c_slave_reg_rx_segment(unsigned int complete, unsigned int size, const uint8_t *p)
{
	// Reset buffer state when switching TX - RX
	if (data.reg.state == RegRead)
		data.reg.buf.state = RegBufInvalid;

	while (size) {
		unsigned int s;
		switch (data.reg.buf.state) {
		case RegBufInvalid:
			i2c_slave_reg_rx_buf();
			break;
		case RegBufComplete:
			if (data.reg.buf.size == data.reg.buf.pos)
				goto done;
			// fall through
		case RegBufSegment:
			if (data.reg.buf.size == data.reg.buf.pos)
				i2c_slave_reg_rx_buf();
			s = data.reg.buf.size - data.reg.buf.pos;
			s = MIN(size, s);
#if DEBUG >= DEBUG_PRINT
			printf(ESC_WRITE "%lu\ti2c reg: Write data segment %u: %u bytes\n",
			       systick_cnt(), data.reg.segment, s);
#endif
			memcpy(&data.reg.buf.p[data.reg.buf.pos], p, s);
			p += s;
			data.reg.buf.pos += s;
			size -= s;
			break;
#if DEBUG >= DEBUG_CHECK
		default:
			dbgbkpt();
			break;
#endif
		}
	}

done:
	if (complete) {
		if (data.reg.state == RegWrite)
			i2c_slave_reg_rx_complete(data.reg.id, data.reg.segment, data.reg.buf.pos, data.reg.buf.p);
		data.reg.buf.state = BufInvalid;
		data.reg.state = RegIdle;
	}
}

static void i2c_slave_reg_tx_buf()
{
	switch (data.reg.state) {
	case RegIdle:
		// Register idle, start read segment
#if DEBUG >= DEBUG_CHECK
		if (data.reg.buf.state != RegBufInvalid)
			dbgbkpt();
#endif
		data.reg.segment = 0;
		data.reg.buf.p = i2c_slave_reg_data(0, data.reg.id, &data.reg.segment, &data.reg.buf.size);
		data.reg.buf.pos = 0;
		data.reg.buf.state = data.reg.segment ? RegBufSegment : RegBufComplete;
		data.reg.state = RegRead;
		break;
	case RegRead:
		// Register read segment complete
#if DEBUG >= DEBUG_CHECK
		if (data.reg.buf.state != RegBufSegment || data.reg.buf.pos != data.reg.buf.size)
			dbgbkpt();
#endif
		data.reg.buf.p = i2c_slave_reg_data(0, data.reg.id, &data.reg.segment, &data.reg.buf.size);
		data.reg.buf.pos = 0;
		data.reg.buf.state = data.reg.segment ? RegBufSegment : RegBufComplete;
		break;
	default:
#if DEBUG >= DEBUG_CHECK
		dbgbkpt();
#endif
		break;
	}

#if DEBUG >= DEBUG_PRINT
	printf(ESC_WRITE "%lu\ti2c reg: Write buffer segment %u: %u bytes\n",
	       systick_cnt(), data.reg.segment, data.reg.buf.size);
#endif
}

static unsigned int i2c_slave_reg_tx_segment(unsigned int *psize, uint8_t *p)
{
	switch (data.reg.state) {
	case RegIdle:
		// Reset pending register buffer state
		data.reg.buf.state = RegBufInvalid;
	case RegRead:
		break;
	default:
#if DEBUG >= DEBUG_CHECK
		dbgbkpt();
#endif
		break;
	}

	unsigned int size = BUF_SIZE;
	do {
		unsigned int s;
		switch (data.reg.buf.state) {
		case RegBufInvalid:
			i2c_slave_reg_tx_buf();
			break;
		case RegBufComplete:
			if (data.reg.buf.size == data.reg.buf.pos)
				goto done;
			// fall through
		case RegBufSegment:
			if (data.reg.buf.size == data.reg.buf.pos)
				i2c_slave_reg_tx_buf();
			s = data.reg.buf.size - data.reg.buf.pos;
			s = MIN(size, s);
#if DEBUG >= DEBUG_PRINT
			printf(ESC_WRITE "%lu\ti2c reg: Write data segment %u: %u bytes\n",
			       systick_cnt(), data.reg.segment, s);
#endif
			memcpy(p, &data.reg.buf.p[data.reg.buf.pos], s);
			p += s;
			data.reg.buf.pos += s;
			size -= s;
			break;
#if DEBUG >= DEBUG_CHECK
		default:
			dbgbkpt();
			break;
#endif
		}
	} while (size);

done:
	*psize = BUF_SIZE - size;
	return data.reg.buf.state == RegBufComplete && data.reg.buf.size == data.reg.buf.pos;
}

static void i2c_slave_process()
{
#if DEBUG >= DEBUG_CHECK
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif
start:	;
	unsigned int bwait = data.buf.intwait;
	unsigned int bufidx = data.buf.procidx;
	switch (data.buf.buf[bufidx].state) {
	case BufTx:
		if (i2c_slave_reg_tx_segment(&data.buf.buf[bufidx].size, data.buf.buf[bufidx].data))
			data.buf.buf[bufidx].state = BufTxComplete;
		else
			data.buf.buf[bufidx].state = BufTxSegment;
		data.buf.procidx = !bufidx;
		goto start;
	case BufRxSegment:
	case BufRxComplete:
#if DEBUG >= DEBUG_PRINT
		printf(ESC_READ "%lu\ti2c: RX %s %u bytes" ESC_DATA, systick_cnt(),
		       data.buf.buf[bufidx].state == BufRxComplete ? "complete" : "segment",
		       data.buf.buf[bufidx].size);
		for (unsigned int i = 0; i < data.buf.buf[bufidx].size; i++)
			printf(" 0x%02x", data.buf.buf[bufidx].data[i]);
		printf("\n");
#endif
		i2c_slave_reg_rx_segment(data.buf.buf[bufidx].state == BufRxComplete,
					 data.buf.buf[bufidx].size, data.buf.buf[bufidx].data);
		data.buf.buf[bufidx].state = BufInvalid;
		data.buf.procidx = !bufidx;
		goto start;
	default:
		break;
	}

	unsigned int wait = data.buf.intwait;
	// Only process pending data buffer if I2C IRQ is waiting
	if (!wait)
		return;
	// Wait for state to become stable
	if (wait != bwait)
		goto start;

#if DEBUG >= DEBUG_CHECK
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
	case BufTxSegment:
	case BufTxComplete:
		break;
	default:
		dbgbkpt();
	}
#endif
	data.buf.intwait = 0;
	// WaitTx state needs manual pending interrupt
	if (data.state == WaitTx)
		NVIC_SetPendingIRQ(I2C2_EV_IRQn);
	// Enable interrupts
	I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk | I2C_CR2_ITERREN_Msk;
}

IDLE_HANDLER() = &i2c_slave_process;
