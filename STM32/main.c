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
	printf(ESC_INIT "%lu\tboot: Initialising peripherals\n", systick_cnt());
	LIST_ITERATE(init, basic_handler_t, p) (*p)();
	printf(ESC_INIT "%lu\tboot: Initialisation done\n", systick_cnt());
}

static void debug()
{
#if 0
	static uint32_t v = 0;
	if ((systick_cnt() - v) >= 1000) {
		printf(ESC_DEBUG "%lu\tdebug: Tick\n", systick_cnt());
		v += 1000;
	}
#endif
}

IDLE_HANDLER(&debug);

int main()
{
	init();
	__enable_irq();

	for (;;) {
		LIST_ITERATE(idle, basic_handler_t, p) (*p)();
		__WFI();
	}
	panic();
}
