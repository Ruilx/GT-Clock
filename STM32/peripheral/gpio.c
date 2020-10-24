#include <stdio.h>
#include <device.h>
#include <debug.h>
#include <system/irq.h>
#include <system/systick.h>
#include "gpio.h"
#include "button.h"

#define DEBUG_PRINT	5
#define DEBUG_POWER	1

#define I2C_DEV_ENA()		GPIOA->BSRR = GPIO_BSRR_BR10_Msk
#define I2C_DEV_CLR()		GPIOA->BSRR = GPIO_BSRR_BS10_Msk
#define DCDC_ENA()		GPIOB->BSRR = GPIO_BSRR_BS0_Msk
#define DCDC_CLR()		GPIOB->BSRR = GPIO_BSRR_BR0_Msk
#define LHT_ENA()		GPIOB->BSRR = GPIO_BSRR_BS1_Msk
#define LHT_CLR()		GPIOB->BSRR = GPIO_BSRR_BR1_Msk

#define ESP_LDO_ENA()		GPIOB->BSRR = GPIO_BSRR_BS4_Msk
#define ESP_LDO_CLR()		GPIOB->BSRR = GPIO_BSRR_BR4_Msk
#define ESP_RESET_ENA()		GPIOB->BSRR = GPIO_BSRR_BR3_Msk
#define ESP_RESET_CLR()		GPIOB->BSRR = GPIO_BSRR_BS3_Msk
#define ESP_GPIO0_SET()		GPIOA->BSRR = GPIO_BSRR_BS15_Msk
#define ESP_GPIO0_CLR()		GPIOA->BSRR = GPIO_BSRR_BR15_Msk

static void gpio_init()
{
	// Configure GPIOs
	// Low speed outputs:
	// PA10: Active-low	I2C device enable
	// PA15: Active-high	ESP GPIO 0
	// PB0:  Active-high	DCDC enable
	// PB1:  Active-high	LHT enable
	// PB3:  Active-low	ESP reset
	// PB4:  Active-high	ESP LDO enable
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN_Msk | RCC_APB2ENR_IOPBEN_Msk;
	DCDC_CLR();
	ESP_LDO_CLR();
	I2C_DEV_CLR();
	LHT_CLR();
	ESP_RESET_CLR();
	ESP_GPIO0_SET();
	GPIOA->CRH = (GPIOA->CRH & ~((GPIO_CRH_CNF10_Msk | GPIO_CRH_MODE10_Msk) |
				     (GPIO_CRH_CNF15_Msk | GPIO_CRH_MODE15_Msk))) |
		     ((0b00 << GPIO_CRH_CNF10_Pos) | (0b10 << GPIO_CRH_MODE10_Pos)) |
		     ((0b00 << GPIO_CRH_CNF15_Pos) | (0b10 << GPIO_CRH_MODE15_Pos));
	GPIOB->CRL = (GPIOB->CRL & ~((GPIO_CRL_CNF0_Msk | GPIO_CRL_MODE0_Msk) |
				     (GPIO_CRL_CNF1_Msk | GPIO_CRL_MODE1_Msk) |
				     (GPIO_CRL_CNF3_Msk | GPIO_CRL_MODE3_Msk) |
				     (GPIO_CRL_CNF4_Msk | GPIO_CRL_MODE4_Msk))) |
		     ((0b00 << GPIO_CRL_CNF0_Pos) | (0b10 << GPIO_CRL_MODE0_Pos)) |
		     ((0b00 << GPIO_CRL_CNF1_Pos) | (0b10 << GPIO_CRL_MODE1_Pos)) |
		     ((0b00 << GPIO_CRL_CNF3_Pos) | (0b10 << GPIO_CRL_MODE3_Pos)) |
		     ((0b00 << GPIO_CRL_CNF4_Pos) | (0b10 << GPIO_CRL_MODE4_Pos));

#if DEBUG >= DEBUG_PRINT
	printf(ESC_INIT "%lu\tgpio: Init done\n", systick_cnt());
#endif
}

INIT_HANDLER() = &gpio_init;

void power_disp_enable(int e)
{
	if (e) {
		DCDC_ENA();
	} else {
		DCDC_CLR();
	}
}

void power_esp_enable(int e)
{
	// Put ESP into reset before we control ESP power supply
	ESP_RESET_ENA();
	if (e) {
		I2C_DEV_ENA();		// TODO Separate I2C power out for STM32 control
		ESP_LDO_ENA();
		ESP_GPIO0_SET();
	} else {
		ESP_LDO_CLR();
		I2C_DEV_CLR();
	}
	systick_delay(2);
	ESP_RESET_CLR();
}

#if DEBUG >= DEBUG_POWER
static void button_handler(uint16_t btn)
{
	static int enable = 0;
	static uint16_t btn_prev = 0;
	if ((btn & ~btn_prev) & ButtonGeneral) {
		enable = !enable;
		power_disp_enable(enable);
		power_esp_enable(enable);
		printf("%s%lu\tgpio: Power %s\n",
		       enable ? ESC_ENABLE : ESC_DISABLE, systick_cnt(),
		       enable ? "enabled", "disabled");
	}
	btn_prev = btn;
}

BUTTON_HANDLER() = &button_handler;
#endif
