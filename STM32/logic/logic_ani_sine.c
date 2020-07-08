// Animation layer: Breathing

#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_animation.h>

static void proc(unsigned int tick)
{
	unsigned int w = 0, h = 0;
	uint8_t *p = matrix_fb(0, &w, &h);
	unsigned int t = tick % 1000;
	for (unsigned int line = 0; line < h; line++) {
		for (unsigned int pix = 0; pix < w; pix++) {
			float x = pix /*- (w - 1) / 2.0*/ - 31.5;
			float y = line - (h - 1) / 2.0;
			float d = sqrt(x * x + y * y) / 16.0 + t / 1000.0;
			float v = (sin(M_PI * 2 * d) + 1.0) / 2.0;
			int iv = (int)(v * 255);
			*(p + line * w + pix) = iv;
		}
	}
}

LOGIC_ANI_HANDLER() = {&proc};
