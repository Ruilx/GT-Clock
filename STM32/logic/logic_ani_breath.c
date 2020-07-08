// Animation layer: Breathing

#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_animation.h>

static void proc(unsigned int tick)
{
	unsigned int w = 0, h = 0;
	uint8_t *p = matrix_fb(0, &w, &h);
	uint8_t v = ((tick & 0x0100) ? 0xff : 0) ^ (tick & 0xff);
	for (unsigned int line = 0; line < h; line++)
		for (unsigned int pix = 0; pix < w; pix++)
			*(p + line * w + pix) = v;
}

//LOGIC_ANI_HANDLER() = {&proc};
