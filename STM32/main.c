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


#if DEBUG
#include <peripheral/matrix.h>

static void debug()
{
	static uint32_t v = 0;
	if ((systick_cnt() - v) >= 1000) {
		// Matrix refresh
		static unsigned int matrix_cnt = 0;
		unsigned int matrix_cnt_now = matrix_refresh_cnt();
		unsigned int matrix_cnt_delta = matrix_cnt_now - matrix_cnt;
		matrix_cnt = matrix_cnt_now;

		printf(ESC_DEBUG "%lu\tdebug: FPS: Matrix %u\n",
		       systick_cnt(), matrix_cnt_delta);
		v += 1000;
	}
}

IDLE_HANDLER(&debug);
#endif

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
