#include <stdio.h>
#include <debug.h>
#include <device.h>
#include <system/systick.h>
#include <system/system.h>

LIST(init, basic_handler_t);
LIST(idle, basic_handler_t);

static inline void init()
{
	printf(ESC_BOOT "%lu\tboot: " VARIANT " build @ " __DATE__ " " __TIME__ "\n", systick_cnt());
	LIST_ITERATE(init, basic_handler_t, p) (*p)();
	printf(ESC_INIT "%lu\tboot: Init done\n", systick_cnt());
}

#if DEBUG > 5
#include <common/common.h>

static void print_id()
{
	char uid[25];
	uid_str(uid);
	printf(ESC_INIT "%lu\tboot: Flash %u KiB, UID %s\n", systick_cnt(),
	       *(uint16_t *)FLASHSIZE_BASE, uid);
}

INIT_HANDLER() = &print_id;
#endif

static void usb_disabled()
{
	// Configure GPIOs
	// PA11 DM: Input pull-down
	// PA12 DP: Input pull-down
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN_Msk;
	GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF11_Msk | GPIO_CRH_MODE11_Msk |
				     GPIO_CRH_CNF12_Msk | GPIO_CRH_MODE12_Msk)) |
		     ((0b10 << GPIO_CRH_CNF11_Pos) | (0b00 << GPIO_CRH_MODE11_Pos)) |
		     ((0b10 << GPIO_CRH_CNF12_Pos) | (0b00 << GPIO_CRH_MODE12_Pos));
	GPIOA->ODR &= ~(GPIO_ODR_ODR11_Msk | GPIO_ODR_ODR12_Msk);

	printf(ESC_DISABLE "%lu\tusb: Disabled\n", systick_cnt());
}

INIT_HANDLER() = &usb_disabled;

#if DEBUG >= 1
static void debug_gpio_init()
{
	// Configure GPIOs
	// PB12: Output push-pull, 50MHz
	// PB14: Output push-pull, 50MHz
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN_Msk;
	GPIOB->CRH = (GPIOB->CRH & ~(GPIO_CRH_CNF12_Msk | GPIO_CRH_CNF12_Msk |
				     GPIO_CRH_CNF14_Msk | GPIO_CRH_MODE14_Msk)) |
		     ((0b00 << GPIO_CRH_CNF12_Pos) | (0b11 << GPIO_CRH_MODE12_Pos)) |
		     ((0b00 << GPIO_CRH_CNF14_Pos) | (0b11 << GPIO_CRH_MODE14_Pos));
	GPIOB->BSRR = GPIO_BSRR_BR12_Msk | GPIO_BSRR_BR14_Msk;
}

INIT_HANDLER() = &debug_gpio_init;
#endif

int main()
{
	__enable_irq();
	init();

	for (;;) {
		LIST_ITERATE(idle, basic_handler_t, p) (*p)();

#if DEBUG >= 1
		// Performance monitor
		static unsigned int pin = 0;
		if (pin)
			GPIOB->BSRR = GPIO_BSRR_BS12_Msk;
		else
			GPIOB->BSRR = GPIO_BSRR_BR12_Msk;
		pin = !pin;
#endif

		//__WFI();
	}
}
