#include <stdio.h>
#include <string.h>
#include <device.h>
#include <debug.h>
#include <system/systick.h>
#include <system/clocks.h>
#include <system/irq.h>
#include "i2c_slave.h"

#define I2C_ADDR	0x1c

#define BUF_SIZE	4

typedef enum {Idle = 0, WaitTx, /*WaitRx,*/ ActiveTx, ActiveRx} state_t;
typedef enum {BufInvalid = 0,
	      BufTx = 2, BufTxSegment = 4, BufTxComplete = 6,
	      BufRx = 3, BufRxSegment = 5, BufRxComplete = 7} buf_t;
typedef enum {RegIdle = 0, RegAddr, RegActive} reg_t;

static struct {
	volatile state_t state;
	struct {
		volatile buf_t type;
		unsigned int size;
		unsigned int pos;
		uint8_t *p;

		//volatile unsigned int dmaact;
		volatile unsigned int dmaidx;
		volatile unsigned int intwait;
		unsigned int procidx;
		unsigned int bufseg;
		struct {
			buf_t state;
			unsigned int size;
			uint8_t data[BUF_SIZE];
		} buf[2];
	} buf;

	struct {
		struct {
			buf_t state;
			unsigned int size;
			unsigned int pos;
			uint8_t *p;
		} buf;

		reg_t state;
		uint8_t id;
		unsigned int segment;
	} reg;
} data = {0};

LIST(i2c_slave_reg, i2c_slave_reg_handler_t);

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

	//i2c_slave_dma_rx();

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

#if DEBUG > 5
	printf(ESC_INIT "%lu\ti2c: Init done\n", systick_cnt());
#endif
}

INIT_HANDLER() = &i2c_slave_init;

static void i2c_slave_irq_rx_wait()
{
	// Wait for RX buffer
	// Disable interrupts for now
	I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
	data.buf.intwait = 1;
}

static void i2c_slave_dma_rx()
{
#if 0
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif
	if (data.buf.buf[data.buf.dmaidx].state != BufInvalid) {
		dbgbkpt();
#if 0
		// Wait for RX buffer
		// Disable interrupts for now
		I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
		data.buf.intwait = 1;
#if 0
		// Wait for RX buffer to become available
		//data.buf.dmaact = 0;
		data.buf.intact = 0;
#endif
#endif
		return;
	}

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
	data.buf.bufseg = 0;
	//data.buf.dmaact = 1;
#if 0
	// Read SR1 once before DR to avoid I2C BTF bug
	__disable_irq();
	I2C2->SR1;
#endif
	// Start RX DMA
	DMA1_Channel5->CCR |= DMA_CCR_EN_Msk;
#if 0
	// ...and again?
	I2C2->SR1;
	__enable_irq();
	return 1;
#endif
}

static inline void i2c_slave_dma_rx_segment()
{
#if 0
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif
	if (data.buf.buf[data.buf.dmaidx].state != BufRx)
		dbgbkpt();

	// Set buffer states
	data.buf.buf[data.buf.dmaidx].size = BUF_SIZE - DMA1_Channel5->CNDTR;
	data.buf.buf[data.buf.dmaidx].state = BufRxSegment;
	data.buf.bufseg = 1;
	// Update double buffering
	data.buf.dmaidx = !data.buf.dmaidx;
	// Disable DMAs
	//DMA1_Channel4->CCR &= ~DMA_CCR_EN_Msk;
	//DMA1_Channel5->CCR &= ~DMA_CCR_EN_Msk;
	// Clear DMA complete flag
	DMA1->IFCR = DMA_IFCR_CTCIF5_Msk | DMA_IFCR_CGIF5_Msk;

	// Don't restart DMA, wait for BTF interrupt to avoid I2C bug
	return;

	// Restart RX DMA if alternative buffer is empty
#if 0
	i2c_slave_dma_rx();
#else
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		i2c_slave_dma_rx();
		break;
	default:
		i2c_slave_irq_rx_wait();
	}
#endif
#if 0
	if (!i2c_slave_dma_rx()) {
		// Wait for RX buffer
		// Disable interrupts for now
		I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
		data.buf.intwait = 1;
	}
#endif
}

static void i2c_slave_dma_rx_done()
{
#if 0
	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;
#endif

#if 0
	if (data.buf.buf[data.buf.dmaidx].state != BufRx)
		dbgbkpt();
#endif
#if 0
	// Ignore if DMA is still waiting and is not a segment
	if (data.buf.bufseg && DMA1_Channel5->CNDTR == BUF_SIZE)
		return;
#endif

	// Set buffer states
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		data.buf.buf[data.buf.dmaidx].size = 0;
		break;
	case BufRx:
		data.buf.buf[data.buf.dmaidx].size = BUF_SIZE - DMA1_Channel5->CNDTR;
		break;
	default:
		dbgbkpt();
	}
	data.buf.buf[data.buf.dmaidx].state = BufRxComplete;
	// Update double buffering
	data.buf.dmaidx = !data.buf.dmaidx;
	// Disable DMAs
	DMA1_Channel4->CCR &= ~DMA_CCR_EN_Msk;
	DMA1_Channel5->CCR &= ~DMA_CCR_EN_Msk;
#if 0
	// Restart RX DMA if alternative buffer is empty
	i2c_slave_dma_rx();
#endif
}

#define EVLOG_SIZE	16
struct {
	unsigned int idx;
#if 0
	uint16_t sr2;
	uint16_t cr1;
	uint16_t cr2;
#endif

	struct {
		uint16_t sr1;
		state_t state;
		reg_t regstate;
		unsigned int dmaidx;
		buf_t dmastate[2];
#if 0
		struct {
			state_t state;
			buf_t type;
			reg_t regstate;
		} new;
#endif
	} log[EVLOG_SIZE];
} evlog = {0};

static void i2c_slave_rx_next()
{
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		i2c_slave_dma_rx();
		break;
	default:
		i2c_slave_irq_rx_wait();
	}
}

static void i2c_slave_rx()
{
#if 1
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		data.state = ActiveRx;
		i2c_slave_dma_rx();
		break;
#if 0
		if (!i2c_slave_dma_rx()) {
			// Wait for RX buffer
			// Disable interrupts for now
			I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
			data.buf.dmawait = 1;
			//data.state = WaitRx;
			data.state = ActiveRx;
			break;
		}
		// fall through
#endif
#if 0
	case BufRx:
		// RX buffer active, DMA active
		data.state = ActiveRx;
		break;
#endif
	default:
		// Wait for RX buffer
		// Disable interrupts for now
		I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
		data.buf.intwait = 1;
		//data.state = WaitRx;
		data.state = ActiveRx;
		break;
	}

#else
	switch (data.buf.type) {
	case BufTx:
	case BufTxSegment:
		data.buf.type = BufTxComplete;
		// fall through
	case BufRxComplete:
	case BufTxComplete:
	case BufInvalid:
		// Wait for RX buffer
#if DEBUG > 5
		printf(ESC_READ "%lu\ti2c irq: RX wait\n", systick_cnt());
#endif
		// Disable interrupts for now
		I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
		data.state = WaitRx;
		break;
	case BufRxSegment:
		if (data.buf.pos == data.buf.size) {
			// Wait for RX buffer
#if DEBUG > 5
			printf(ESC_READ "%lu\ti2c irq: RX wait\n", systick_cnt());
#endif
			// Disable interrupts for now
			I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
			data.state = WaitRx;
			break;
		}
		// fall through
	case BufRx:
		// RX buffer valid or empty
		if (I2C2->SR1 & I2C_SR1_RXNE_Msk) {
			// NAK if no more buffer space available
			// No, slave does not NAK
			//if (data.buf.size <= 1)
			//	I2C2->CR1 &= ~I2C_CR1_ACK_Msk;
			// Read RX buffer
			uint8_t v = I2C2->DR;
			evlog.log[evlog.idx].dr = v;
#if DEBUG > 5
			printf(ESC_READ "%lu\ti2c irq: RX %s" ESC_DATA "0x%02x\n", systick_cnt(),
			       data.buf.size != data.buf.pos ? "" : ESC_ERROR "dropped ", v);
#endif
			if (data.buf.size != data.buf.pos) {
				*(data.buf.p + data.buf.pos) = v;
				data.buf.pos++;
			}
		}
		if (data.buf.type == BufRxSegment && data.buf.size == data.buf.pos) {
			// Wait for RX buffer
			// Disable interrupts for now
			I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
			data.state = WaitRx;
		} else {
			// Enable interrupts
			I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk;
			data.state = ActiveRx;
		}
	}
#endif
}

static void i2c_slave_done()
{
#if DEBUG > 5
	printf(ESC_DISABLE "%lu\ti2c irq: stop\n", systick_cnt());
#endif

#if 1
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
	case BufRx:
		i2c_slave_dma_rx_done();
		data.state = Idle;
		// Re-enable ACK and clear stop flag
		I2C2->CR1 |= I2C_CR1_ACK_Msk;
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk;
		break;
	default:
		// Need to wait for RX buffer for end indication
		// Disable interrupts for now
		I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
		data.buf.intwait = 1;
		//data.state = WaitRx;
		//data.state = ActiveRx;
		break;
	}
#else
	switch (data.buf.type) {
	case BufTx:
	case BufTxSegment:
		data.buf.type = BufTxComplete;
		break;
	case BufRx:
	case BufRxSegment:
		data.buf.type = BufRxComplete;
		break;
	default:
		break;
	}
#endif
}

static void i2c_slave_addr()
{
	// Set up RX DMA buffer before acknowledging ADDR
	// ...to avoid I2C bug with RX DR overrun
	switch (data.buf.buf[data.buf.dmaidx].state) {
	case BufInvalid:
		i2c_slave_dma_rx();
		break;
	default:
		i2c_slave_irq_rx_wait();
		return;
	}
	// Addressed, switch to transmitter or receiver mode
	uint16_t sr2 = I2C2->SR2;
#if DEBUG > 5
	printf(ESC_ENABLE "%lu\ti2c irq: addr 0x%04x\n", systick_cnt(), sr2);
#endif
	// Already in receiver mode, switch to transmitter if necessary
	if (sr2 & I2C_SR2_TRA_Msk)
		dbgbkpt();
	else
		data.state = ActiveRx;
#if 0
	else
		i2c_slave_rx();
#endif
#if 0
	if (sr2 & I2C_SR2_TRA_Msk)
		i2c_slave_tx();
	else
		i2c_slave_rx();
#endif
}

void DMA1_Channel4_IRQHandler()
{
	dbgbkpt();
}

void DMA1_Channel5_IRQHandler()
{
	GPIOB->BSRR = GPIO_BSRR_BS14_Msk;
	i2c_slave_dma_rx_segment();
	GPIOB->BSRR = GPIO_BSRR_BR14_Msk;
}

void I2C2_EV_IRQHandler()
{
	GPIOB->BSRR = GPIO_BSRR_BS12_Msk;
	uint16_t sr1 = I2C2->SR1;
	if (!sr1) {
		GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
		return;
	}

#if DEBUG > 5
	printf(ESC_DEBUG "%lu\ti2c irq: SR1 0x%04x\n", systick_cnt(), sr1);
	//flushCache();
#endif
#if DEBUG > 4
	evlog.log[evlog.idx].sr1 = sr1;
	evlog.log[evlog.idx].state = data.state;
	evlog.log[evlog.idx].dmaidx = data.buf.dmaidx;
	evlog.log[evlog.idx].dmastate[0] = data.buf.buf[0].state;
	evlog.log[evlog.idx].dmastate[1] = data.buf.buf[1].state;
	evlog.log[evlog.idx].regstate = data.reg.state;
	//evlog.log[evlog.idx].dr = 0xaa;
#endif

	I2C_TypeDef *i2c = I2C2;
	DMA_TypeDef *dma = DMA1;
	DMA_Channel_TypeDef *dmarx = DMA1_Channel5;
	DMA_Channel_TypeDef *dmatx = DMA1_Channel4;

	// For slave mode, possible event interrupts are:
	// ADDR		Address matched
	// STOPF	Stop condition
	// BTF only	???
	// TXE		Transmit buffer empty
	// RXNE		Receive buffer not empty

	switch (data.state) {
	case Idle:
		if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
		else if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
#if 0
		if (sr1 & I2C_SR1_RXNE_Msk)
			i2c_slave_rx();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
		else if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
#endif
		else
			dbgbkpt();
		break;
	case WaitTx:
		break;
	case ActiveTx:
#if 0
		if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
		else if (sr1 & I2C_SR1_TXE_Msk)
			i2c_slave_tx();
		else
#endif
			dbgbkpt();
		break;
	//case WaitRx:
	//	break;
	case ActiveRx:
		if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else if(sr1 & I2C_SR1_BTF_Msk)
			i2c_slave_rx_next();
#if 0
		if (sr1 & I2C_SR1_RXNE_Msk)
			i2c_slave_rx();
		else if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
#endif
		else
			dbgbkpt();
		break;
	}

#if DEBUG > 5
	evlog.log[evlog.idx].new.state = data.state;
	evlog.log[evlog.idx].new.type = data.buf.type;
	evlog.log[evlog.idx].new.regstate = data.reg.state;
#endif
#if DEBUG > 4
	evlog.idx = evlog.idx == EVLOG_SIZE - 1 ? 0 : evlog.idx + 1;
#endif
	GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
}

static void i2c_slave_tx()
{
	switch (data.buf.type) {
	case BufRx:
	case BufRxSegment:
		data.buf.type = BufRxComplete;
		// fall through
	case BufRxComplete:
	case BufTxComplete:
	case BufInvalid:
		// Wait for TX buffer
#if DEBUG > 5
		printf(ESC_WRITE "%lu\ti2c irq: TX wait\n", systick_cnt());
#endif
		// Disable interrupts for now
		I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
		data.state = WaitTx;
		break;
	case BufTxSegment:
		if (data.buf.pos == data.buf.size) {
			// Wait for TX buffer
#if DEBUG > 5
			printf(ESC_WRITE "%lu\ti2c irq: TX segment wait\n", systick_cnt());
#endif
			// Disable interrupts for now
			I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
			data.state = WaitTx;
			break;
		}
		// fall through
	case BufTx:
		// TX buffer valid or empty
		if (I2C2->SR1 & I2C_SR1_TXE_Msk) {
			uint8_t v = 0;
#if DEBUG > 5
			printf(ESC_WRITE "%lu\ti2c irq: TX %s" ESC_DATA, systick_cnt(),
			       data.buf.size != data.buf.pos ? "" : ESC_ERROR "invalid ");
#endif
			if (data.buf.size != data.buf.pos) {
				v = *(data.buf.p + data.buf.pos);
				data.buf.pos++;
			}
#if DEBUG > 5
			printf("0x%02x\n", v);
#endif
			I2C2->DR = v;
		}
		if (data.buf.type == BufTxSegment && data.buf.pos == data.buf.size) {
			// Wait for TX buffer
			// Disable interrupts for now
			I2C2->CR2 &= ~I2C_CR2_ITEVTEN_Msk;
			data.state = WaitTx;
		} else {
			// Enable interrupts
			I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk;
			data.state = ActiveTx;
		}
	}
}

void I2C2_ER_IRQHandler()
{
	uint16_t sr1 = I2C2->SR1;

#if DEBUG > 5
	printf(ESC_DEBUG "%lu\ti2c err irq: SR1 0x%04x\n", systick_cnt(), sr1);
	//flushCache();
#endif

	if (sr1 & I2C_SR1_AF_Msk) {
		// No acknowledgement
#if DEBUG > 5
		printf(ESC_ERROR "%lu\ti2c err irq: nak\n", systick_cnt());
#endif
		i2c_slave_done();
		I2C2->SR1 = ~I2C_SR1_AF_Msk;
	} else if (sr1 & I2C_SR1_BERR_Msk) {
		// Bus error
#if DEBUG > 5
		printf(ESC_ERROR "%lu\ti2c err irq: bus error\n", systick_cnt());
#endif
		I2C2->SR1 = ~I2C_SR1_BERR_Msk;
	} else {
		dbgbkpt();
	}
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

#if DEBUG > 5
	printf(ESC_WRITE "%lu\ti2c reg: Write complete 0x%02x, segment %u: %u bytes\n",
	       systick_cnt(), data.reg.id, data.reg.segment, data.reg.buf.size);
#endif
}

static inline void i2c_slave_reg_rx_buf()
{
	switch (data.reg.state) {
	case RegIdle:
		// Register idle, wait for ID
		if (data.reg.buf.state != BufInvalid)
			dbgbkpt();
		data.reg.buf.p = &data.reg.id;
		data.reg.buf.size = 1;
		data.reg.buf.pos = 0;
		data.reg.buf.state = BufRxSegment;
		data.reg.state = RegAddr;
		break;
	case RegAddr:
		// Register ID received
		if (data.reg.buf.state != BufRxSegment || data.reg.buf.pos != 1)
			dbgbkpt();
		data.reg.segment = 0;
		data.reg.buf.p = i2c_slave_reg_data(1, data.reg.id, &data.reg.segment, &data.reg.buf.size);
		data.reg.buf.pos = 0;
		data.reg.buf.state = data.reg.segment ? BufRxSegment : BufRx;
		data.reg.state = RegActive;
		break;
	case RegActive:
		// Register write segment complete
		if (data.reg.buf.state != BufRxSegment || data.reg.buf.pos != data.reg.buf.size)
			dbgbkpt();
		data.reg.buf.p = i2c_slave_reg_data(1, data.reg.id, &data.reg.segment, &data.reg.buf.size);
		data.reg.buf.pos = 0;
		data.reg.buf.state = data.reg.segment ? BufRxSegment : BufRx;
		break;
	default:
		dbgbkpt();
	}

#if DEBUG > 5
	printf(ESC_WRITE "%lu\ti2c reg: Write buffer segment %u: %u bytes\n",
	       systick_cnt(), data.reg.segment, data.reg.buf.size);
#endif
}

static inline void i2c_slave_reg_rx_segment(unsigned int complete, unsigned int size, const uint8_t *p)
{
	while (size) {
		unsigned int s;
		switch (data.reg.buf.state) {
		case BufInvalid:
			i2c_slave_reg_rx_buf();
			break;
		case BufRx:
			if (data.reg.buf.size == data.reg.buf.pos)
				goto done;
			// fall through
		case BufRxSegment:
			if (data.reg.buf.size == data.reg.buf.pos)
				i2c_slave_reg_rx_buf();
			s = data.reg.buf.size - data.reg.buf.pos;
			s = MIN(size, s);
#if DEBUG > 5
			printf(ESC_WRITE "%lu\ti2c reg: Write data segment %u: %u bytes\n",
			       systick_cnt(), data.reg.segment, s);
#endif
			memcpy(&data.reg.buf.p[data.reg.buf.pos], p, s);
			p += s;
			size -= s;
			data.reg.buf.pos += s;
			break;
		default:
			dbgbkpt();
			break;
		}
	}

done:
	if (complete) {
		if (data.reg.state == RegActive)
			i2c_slave_reg_rx_complete(data.reg.id, data.reg.segment, data.reg.buf.size, data.reg.buf.p);
		data.reg.buf.state = BufInvalid;
		data.reg.state = RegIdle;
	}
}

static inline void i2c_slave_reg_tx_buf()
{
	if (data.buf.type == BufInvalid)
		data.reg.segment = 0;
	else if (data.buf.type != BufTxSegment || data.buf.pos != data.buf.size)
		dbgbkpt();
	data.buf.type = BufInvalid;
	data.buf.p = i2c_slave_reg_data(0, data.reg.id, &data.reg.segment, &data.buf.size);
	data.buf.pos = 0;
	data.buf.type = data.reg.segment ? BufTxSegment : BufTx;
}

static void i2c_slave_process()
{
start:	;
	unsigned int bwait = data.buf.intwait;
	//state_t bstate = data.state;
	unsigned int bufidx = data.buf.procidx;
	switch (data.buf.buf[bufidx].state) {
	case BufTxComplete:
		dbgbkpt();
#if DEBUG > 5
		printf(ESC_WRITE "%lu\ti2c: TX complete %u bytes" ESC_DATA,
		       systick_cnt(), data.buf.pos);
		for (unsigned int i = 0; i < data.buf.pos; i++)
			printf(" 0x%02x", data.buf.p[i]);
		printf("\n");
#endif
		data.buf.type = BufInvalid;
		data.buf.procidx = !bufidx;
		goto start;
	case BufRxSegment:
	case BufRxComplete:
#if DEBUG > 5
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
	//state_t state = data.state;
	// Only process pending data buffer if I2C IRQ is waiting
	if (!wait /*|| state != ActiveRx*/ /*state != WaitTx && state != WaitRx*/)
		return;
	// Wait for state to become stable
	//if (state != bstate)
	//	goto start;
	if (wait != bwait)
		goto start;

#if 0
	switch (state) {
	case WaitTx:
		dbgbkpt();
#if 0
#if DEBUG > 4
		printf(ESC_WRITE "%lu\ti2c: TX buffer\n", systick_cnt());
#endif
		i2c_slave_reg_tx_buf();
		data.state = ActiveTx;
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk;
#endif
		break;
	case ActiveRx:
#if 0
#if DEBUG > 4
		printf(ESC_READ "%lu\ti2c: RX buffer\n", systick_cnt());
#endif
		i2c_slave_reg_rx_buf();
#endif
#endif
		if (data.buf.buf[data.buf.dmaidx].state != BufInvalid)
			dbgbkpt();
		//data.state = ActiveRx;
		data.buf.intwait = 0;
		//i2c_slave_rx();
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITEVTEN_Msk;
#if 0
		break;
	default:
		dbgbkpt();
	}
#endif
}

IDLE_HANDLER() = &i2c_slave_process;
