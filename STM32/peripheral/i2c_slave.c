#include <stdio.h>
#include <device.h>
#include <debug.h>
#include <system/systick.h>
#include <system/clocks.h>
#include <system/irq.h>

#define I2C_ADDR	0x1c

typedef enum {Idle = 0, WaitTx, WaitRx, ActiveTx, ActiveRx} state_t;
typedef enum {BufInvalid = 0,
	      BufTx = 2, BufTxSegment = 4, BufTxComplete = 6,
	      BufRx = 3, BufRxSegment = 5, BufRxComplete = 7} buf_t;
typedef enum {RegIdle = 0, RegAddr, RegActive} reg_t;

static struct {
	volatile state_t state;
	struct {
		buf_t type;
		unsigned int size;
		unsigned int pos;
		uint8_t *p;
	} buf;
	struct {
		reg_t state;
		uint8_t id;
		unsigned int segment;
	} reg;
} data = {0};

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
	// DMA disabled, buffer interrupts enabled, event interrupts enabled
	// Set APB clock frequency
	I2C2->CR2 = I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk | I2C_CR2_ITERREN_Msk |
		    ((clkAPB1() / 1000000) << I2C_CR2_FREQ_Pos);
	// Configure I2C address, 7-bit mode
	I2C2->OAR1 = (I2C_ADDR << I2C_OAR1_ADD1_Pos);
	// Dual address disabled
	I2C2->OAR2 = 0;
	// Clear flags
	I2C2->SR1 = 0;
	// Clock control, 400kHz fast mode
	I2C2->CCR = I2C_CCR_FS_Msk | I2C_CCR_DUTY_Msk | (4 << I2C_CCR_CCR_Pos);
	// Configure interrupts
	uint32_t pg = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(I2C2_EV_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_I2C, 0));
	NVIC_EnableIRQ(I2C2_EV_IRQn);
	NVIC_SetPriority(I2C2_ER_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_I2C, 0));
	NVIC_EnableIRQ(I2C2_ER_IRQn);
	// Enable I2C peripheral
	I2C2->CR1 = I2C_CR1_PE_Msk;

	// ACK enable
	I2C2->CR1 = I2C_CR1_PE_Msk | I2C_CR1_ACK_Msk;

	printf(ESC_INIT "%lu\ti2c: Initialisation done\n", systick_cnt());
}

INIT_HANDLER(&i2c_slave_init);

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
		I2C2->CR2 &= ~(I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk);
		data.state = WaitTx;
		break;
	case BufTxSegment:
		if (data.buf.pos == data.buf.size) {
			// Wait for TX buffer
#if DEBUG > 5
			printf(ESC_WRITE "%lu\ti2c irq: TX segment wait\n", systick_cnt());
#endif
			// Disable interrupts for now
			I2C2->CR2 &= ~(I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk);
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
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk;
		data.state = ActiveTx;
	}
}

static void i2c_slave_rx()
{
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
		I2C2->CR2 &= ~(I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk);
		data.state = WaitRx;
		break;
	case BufRxSegment:
		if (data.buf.pos == data.buf.size) {
			// Wait for RX buffer
#if DEBUG > 5
			printf(ESC_READ "%lu\ti2c irq: RX wait\n", systick_cnt());
#endif
			// Disable interrupts for now
			I2C2->CR2 &= ~(I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk);
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
#if DEBUG > 5
			printf(ESC_READ "%lu\ti2c irq: RX %s" ESC_DATA "0x%02x\n", systick_cnt(),
			       data.buf.size != data.buf.pos ? "" : ESC_ERROR "dropped ", v);
#endif
			if (data.buf.size != data.buf.pos) {
				*(data.buf.p + data.buf.pos) = v;
				data.buf.pos++;
			}
		}
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk;
		data.state = ActiveRx;
	}
}

static void i2c_slave_done()
{
#if DEBUG > 5
	printf(ESC_DISABLE "%lu\ti2c irq: stop\n", systick_cnt());
#endif

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
	data.state = Idle;

	// Re-enable ACK and clear stop flag
	I2C2->CR1 |= I2C_CR1_ACK_Msk;
	// Enable interrupts
	I2C2->CR2 |= I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk;
}

static void i2c_slave_addr()
{
	// Addressed, switch to transmitter or receiver mode
	uint8_t sr2 = I2C2->SR2;
#if DEBUG > 5
	printf(ESC_ENABLE "%lu\ti2c irq: addr 0x%04x\n", systick_cnt(), sr2);
#endif

	if (sr2 & I2C_SR2_TRA_Msk)
		i2c_slave_tx();
	else
		i2c_slave_rx();
}

void I2C2_EV_IRQHandler()
{
	uint16_t sr1 = I2C2->SR1;

#if DEBUG > 5
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
		if (sr1 & I2C_SR1_RXNE_Msk)
			i2c_slave_rx();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
		else if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else
			dbgbkpt();
		break;
	case WaitTx:
	case ActiveTx:
		if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
		else if (sr1 & I2C_SR1_TXE_Msk)
			i2c_slave_tx();
		else
			dbgbkpt();
		break;
	case WaitRx:
	case ActiveRx:
		if (sr1 & I2C_SR1_RXNE_Msk)
			i2c_slave_rx();
		else if (sr1 & I2C_SR1_STOPF_Msk)
			i2c_slave_done();
		else if (sr1 & I2C_SR1_ADDR_Msk)
			i2c_slave_addr();
		else
			dbgbkpt();
		break;
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

#include <peripheral/matrix.h>
static uint8_t regs[256];

enum {FuncX = 0x10, FuncY = 0x11, FuncV = 0x12, FuncP = 0x13};

static void *i2c_slave_reg_data(uint8_t id, unsigned int *segment, unsigned int *size)
{
	if (id == FuncP) {
		unsigned int w = 0, h = 0;
		unsigned int x = regs[FuncX], y = regs[FuncY];
		uint8_t *p = matrix_fb(&w, &h);
		unsigned int ofs = y * w + x;
		p += ofs;
		*segment = 0;
		*size = ofs >= w * h ? 0 : w * h - ofs;
		return p;
	}

	*segment = 0;
	*size = 1;
	return &regs[id];
}

static void *i2c_slave_reg_read(uint8_t id, unsigned int *segment, unsigned int *size)
{
	return i2c_slave_reg_data(id, segment, size);
#if 0
	printf(ESC_READ "%lu\ti2c reg: read 0x%02x, segment %u, %u bytes\n",
	       systick_cnt(), id, *segment, *size);
#endif
}

static void *i2c_slave_reg_write(uint8_t id, unsigned int *segment, unsigned int *size)
{
	return i2c_slave_reg_data(id, segment, size);
#if 0
	printf(ESC_WRITE "%lu\ti2c reg: write 0x%02x, segment %u, %u bytes\n",
	       systick_cnt(), id, *segment, *size);
#endif
}

static void i2c_slave_reg_write_complete(uint8_t id, unsigned int segment, unsigned int size, void *p)
{
	if (id == FuncV)
		matrix_set_pixel(regs[FuncX], regs[FuncY], regs[FuncV]);
#if DEBUG > 5
	uint8_t *ptr = p;
	printf(ESC_WRITE "%lu\ti2c reg: write 0x%02x complete, segment %u, %u bytes" ESC_DATA,
	       systick_cnt(), id, segment, size);
	for (unsigned int i = 0; i < data.buf.pos; i++)
		printf(" 0x%02x", ptr[i]);
	printf("\n");
#endif
}

static inline void i2c_slave_reg_rx_buf()
{
	switch (data.reg.state) {
	case RegIdle:
		// Register idle, wait for ID
		if (data.buf.type != BufInvalid)
			dbgbkpt();
		data.buf.p = &data.reg.id;
		data.buf.size = 1;
		data.buf.pos = 0;
		data.buf.type = BufRxSegment;
		data.reg.state = RegAddr;
		break;
	case RegAddr:
		// Register ID received
		if (data.buf.type != BufRxSegment || data.buf.pos != 1)
			dbgbkpt();
		data.reg.segment = 0;
		data.buf.p = i2c_slave_reg_write(data.reg.id, &data.reg.segment, &data.buf.size);
		data.buf.pos = 0;
		data.buf.type = data.reg.segment ? BufRxSegment : BufRx;
		data.reg.state = RegActive;
		break;
	case RegActive:
		// Register write segment complete
		if (data.buf.type != BufRxSegment || data.buf.pos != data.buf.size)
			dbgbkpt();
		data.buf.p = i2c_slave_reg_write(data.reg.id, &data.reg.segment, &data.buf.size);
		data.buf.pos = 0;
		data.buf.type = data.reg.segment ? BufRxSegment : BufRx;
		break;
	default:
		dbgbkpt();
	}
}

static inline void i2c_slave_reg_rx_complete()
{
	i2c_slave_reg_write_complete(data.reg.id, data.reg.segment, data.buf.pos, data.buf.p);
	data.reg.state = RegIdle;
	data.buf.type = BufInvalid;
}

static inline void i2c_slave_reg_tx_buf()
{
	if (data.buf.type == BufInvalid)
		data.reg.segment = 0;
	else if (data.buf.type != BufTxSegment || data.buf.pos != data.buf.size)
		dbgbkpt();
	data.buf.p = i2c_slave_reg_read(data.reg.id, &data.reg.segment, &data.buf.size);
	data.buf.pos = 0;
	data.buf.type = data.reg.segment ? BufTxSegment : BufTx;
}

static void i2c_slave_process()
{
	I2C_TypeDef *i2c = I2C2;

	static uint8_t buf[8];

	switch (data.buf.type) {
	case BufTxComplete:
#if DEBUG > 5
		printf(ESC_WRITE "%lu\ti2c: TX complete %u bytes" ESC_DATA,
		       systick_cnt(), data.buf.pos);
		for (unsigned int i = 0; i < data.buf.pos; i++)
			printf(" 0x%02x", data.buf.p[i]);
		printf("\n");
#endif
		data.buf.type = BufInvalid;
		break;
	case BufRxComplete:
#if DEBUG > 5
		printf(ESC_READ "%lu\ti2c: RX complete %u bytes" ESC_DATA,
		       systick_cnt(), data.buf.pos);
		for (unsigned int i = 0; i < data.buf.pos; i++)
			printf(" 0x%02x", data.buf.p[i]);
		printf("\n");
#endif
		i2c_slave_reg_rx_complete();
		break;
	default:
		break;
	}

	switch (data.state) {
	case WaitTx:
#if DEBUG > 5
		printf(ESC_WRITE "%lu\ti2c: TX buffer\n", systick_cnt());
#endif
		i2c_slave_reg_tx_buf();
		data.state = ActiveTx;
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk;
		break;
	case WaitRx:
#if DEBUG > 5
		printf(ESC_READ "%lu\ti2c: RX buffer\n", systick_cnt());
#endif
		i2c_slave_reg_rx_buf();
		data.state = ActiveRx;
		// Enable interrupts
		I2C2->CR2 |= I2C_CR2_ITBUFEN_Msk | I2C_CR2_ITEVTEN_Msk;
		break;
	default:
		break;
	}
}

IDLE_HANDLER(&i2c_slave_process);
