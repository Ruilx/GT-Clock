// Animation layer: Breathing

#include <stdlib.h>
#include <math.h>
#include <debug.h>
//#include <device.h>
#include <peripheral/matrix.h>
#include <logic/logic_animation.h>

#define LUT_W	40
#define LUT_H	8

static struct {
	// Distance LUT
	uint8_t lut[LUT_H][LUT_W];
	uint8_t sin[256];
} data;

static void updateLut(float ox, float oy, float period)
{
	for (unsigned int iy = 0; iy < LUT_H; iy++) {
		for (unsigned int ix = 0; ix < LUT_W; ix++) {
			float x = (float)ix - ox;
			float y = (float)iy - oy;
			float d = sqrt(x * x + y * y);
			float t = d / 16.0;
			t = t - floor(t);
			data.lut[iy][ix] = round(t * 255.0);
		}
	}

	const unsigned int s = sizeof(data.sin);
	for (unsigned int ix = 0; ix < s; ix++) {
		float x = ix;
		float t = x / 255.0 * 2.0 - 1.0;
		float v = (sin(period * t) + 1.0) / 2.0;
		data.sin[ix] = round(v * 255.0);
	}
}

static void init()
{
	updateLut(31.5, 3.5, M_TWOPI);
}

INIT_HANDLER() = &init;

static void proc(unsigned int tick)
{
	const unsigned int s = sizeof(data.sin);
	unsigned int t = (tick % 1000) * (s - 1) / 999;

	unsigned int w = 0, h = 0;
	uint8_t *p = matrix_fb(0, &w, &h);

	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			uint8_t v = data.lut[y][x];
			v = data.sin[(v - t) % s];
			*(p + y * w + x) = v;
		}
	}
}

LOGIC_ANI_HANDLER() = {&proc};
