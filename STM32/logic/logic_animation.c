#include <debug.h>
#include <system/systick.h>
#include <peripheral/matrix.h>
#include "logic_animation.h"

static struct {
	unsigned int enable;
	unsigned int refcnt;
} data;

LIST(logic_ani, logic_ani_handler_t);

void logic_animation_enable(unsigned int e)
{
	data.enable = !!e;
}

unsigned int logic_animation_refresh_cnt()
{
	return data.refcnt;
}

static void proc()
{
	static unsigned int _tick = 0;
	uint32_t tick = systick_cnt();
	if (_tick == tick)
		return;

	_tick = tick;
	if (!data.enable)
		return;

	LIST_ITERATE(logic_ani, logic_ani_handler_t, phdr)
		phdr->proc(tick);

	matrix_fb_swap();
	data.refcnt++;
}

IDLE_HANDLER(&proc);
