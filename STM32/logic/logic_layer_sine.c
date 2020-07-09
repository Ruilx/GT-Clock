// Layer: Sine wave
// Parameters:
//   [0]	Flags
//     0x01	Half-aligned to left top corner of selected pixel
//   [1]	X centre offset
//   [2]	Y centre offset
//   [3]	Multiply factor of original value
//   [4]	Multiply factor of new value
//   [5]	Multiply factor of mixer value
//   [6]	Multiply factor of spacial sine period
//   [7]	Division factor of spacial sine period
//   [8]	Multiply factor of temporal sine period
//   [9]	Division factor of temporal sine period

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

#define SIN_LUT_SIZE	256

typedef struct PACKED {
	uint8_t flags;
	uint8_t x;
	uint8_t y;
	uint8_t m[3];
	struct {
		uint8_t mult;
		uint8_t div;
	} space, time;
} param_t;

static void updateLut(float ox, float oy, float period, uint8_t *ptr)
{
	uint8_t *psin = ptr;
	uint8_t *plut = ptr + SIN_LUT_SIZE;
	unsigned int w = 0, h = 0;
	matrix_fb(0, &w, &h);

	for (unsigned int iy = 0; iy < h; iy++) {
		for (unsigned int ix = 0; ix < w; ix++) {
			float x = (float)ix - ox;
			float y = (float)iy - oy;
			float d = sqrt(x * x + y * y);
			float t = d / 16.0;
			t = t - floor(t);
			plut[iy * w + ix] = round(t * 255.0);
		}
	}

	for (unsigned int ix = 0; ix < SIN_LUT_SIZE; ix++) {
		float x = ix;
		float t = x / 255.0 * 2.0 - 1.0;
		float v = (sin(period * t) + 1.0) / 2.0;
		psin[ix] = round(v * 255.0);
	}
}

static void *config(void *param, unsigned int *ok)
{
	param_t *pp = param;
	float v = (pp->flags & 0x01) ? -0.5 : 0;
	float x = (float)pp->x + v;
	float y = (float)pp->y + v;
	float m = pp->space.mult;
	float d = pp->space.div == 0 ? 1 : pp->space.div;

	unsigned int w, h;
	matrix_fb(0, &w, &h);
	void *ptr = logic_layers_alloc(SIN_LUT_SIZE + w * h);
	if (ptr == 0)
		*ok = 0;
	else
		updateLut(x, y, M_TWOPI * m / d, ptr);
	return ptr;
}

static void proc(unsigned int tick, void *param, void *ptr)
{
	if (!ptr)
		return;

	param_t *pp = param;
	uint8_t *psin = ptr;
	uint8_t *plut = ptr + SIN_LUT_SIZE;

	unsigned int w = 0, h = 0;
	uint8_t *p = matrix_fb(0, &w, &h);

	tick = tick * pp->time.mult / pp->time.div;
	unsigned int t = (tick % 1000) * (SIN_LUT_SIZE - 1) / 999;

	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			uint8_t *pv = p + y * w + x;
			unsigned int a = *pv;
			unsigned int b = plut[y * w + x];
			b = psin[(b - t) % SIN_LUT_SIZE];
			*pv = ((a * pp->m[0] * 255) + (b * pp->m[1] * 255) + (a * b * pp->m[2])) / (255 * 255);
		}
	}
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdSine,
	.config = &config,
	.proc = &proc,
};
