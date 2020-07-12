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

int main()
{
	__enable_irq();
	init();

	for (;;) {
		LIST_ITERATE(idle, basic_handler_t, p) (*p)();
		__WFI();
	}
}
