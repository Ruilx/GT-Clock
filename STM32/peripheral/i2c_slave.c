#include <stdio.h>
#include <device.h>
#include <debug.h>
#include <system/systick.h>
#include <system/clocks.h>
#include <system/irq.h>

#define I2C_ADDR	0x1c

I2C_TypeDef *i2c = I2C2;

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
	// DMA disabled, interrupts enabled, set APB clock frequency
	I2C2->CR2 = I2C_CR2_ITEVTEN_Msk | I2C_CR2_ITERREN_Msk |
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

void I2C2_EV_IRQHandler()
{
	static I2C_TypeDef *i2c = I2C2;

	uint16_t sr1 = I2C2->SR1;
	unsigned int processed = 0;

	printf(ESC_DEBUG "%lu\ti2c: irq 0x%04x\n", systick_cnt(), sr1);

	if (sr1 & I2C_SR1_RXNE_Msk) {
		// Byte received
		uint8_t v = I2C2->DR;
		printf(ESC_GREEN "%lu\ti2c: read: 0x%02x\n", systick_cnt(), v);
		processed = 1;
	}
	if (sr1 & I2C_SR1_STOPF_Msk) {
		// Stop condition
		printf(ESC_BLUE "%lu\ti2c: stop\n", systick_cnt());
		// Set ACK again
		I2C2->SR1;
		I2C2->CR1 = I2C_CR1_PE_Msk | I2C_CR1_ACK_Msk;
		processed = 1;
	}
	if (sr1 & I2C_SR1_ADDR_Msk) {
		// Addressed
		printf(ESC_YELLOW "%lu\ti2c: addr\n", systick_cnt());
		I2C2->SR1;
		uint16_t sr2 = I2C2->SR2;
		processed = 1;
	}
	if (sr1 & I2C_SR1_TXE_Msk) {
		// TX empty
		printf(ESC_RED "%lu\ti2c: write\n", systick_cnt());
		I2C2->DR = 0x39;
		processed = 1;
	}

	//flushCache();

	if (!processed)
		dbgbkpt();
}

void I2C2_ER_IRQHandler()
{
	uint16_t sr1 = I2C2->SR1;
	uint16_t sr2 = I2C2->SR2;
	unsigned int processed = 0;

	printf(ESC_DEBUG "%lu\ti2c: irq err 0x%04x 0x%04x\n", systick_cnt(), sr1, sr2);

	if (sr1 & I2C_SR1_BERR_Msk) {
		// Bus error
		printf(ESC_ERROR "%lu\ti2c: bus error\n", systick_cnt());
		I2C2->SR1 = ~I2C_SR1_BERR_Msk;
		processed = 1;
	}
	if (sr1 & I2C_SR1_AF_Msk) {
		// No acknowledgement
		printf(ESC_ERROR "%lu\ti2c: nak\n", systick_cnt());
		I2C2->SR1 = ~I2C_SR1_AF_Msk;
		processed = 1;
	}

	//flushCache();

	if (!processed)
		dbgbkpt();
}
