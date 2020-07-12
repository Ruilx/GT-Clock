// Layer: Sine wave
// Parameters:
//   [0]	Flags
//     0x04	Reversed half-period sine wave
//     0x02	Half-period sine wave
//     0x01	Half-aligned to left top corner of selected pixel
//   [1]	X centre offset
//   [2]	Y centre offset
//   [3]	Multiply factor of spacial sine period
//   [4]	Division factor of spacial sine period
//   [5]	Multiply factor of temporal sine period
//   [6]	Division factor of temporal sine period

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

#define SIN_LUT_SIZE	256

enum {FlagHalfAligned = 0x01, FlagHalfPeriod = 0x02, FlagReversedHalfPeriod = 0x04};

typedef struct PACKED {
	uint8_t flags;
	int8_t x;
	int8_t y;
	struct {
		uint8_t mult;
		uint8_t div;
	} space, time;
} param_t;

typedef struct PACKED {
	uint8_t lut_sin[256];
	uint8_t lut_dis[];
} data_t;

static inline void updateLut(param_t *pp, data_t *pdata, unsigned int w, unsigned int h)
{
	float v = pp->flags & FlagHalfAligned ? -0.5 : 0;
	float m = pp->space.mult;
	float d = pp->space.div == 0 ? 1 : pp->space.div;
	float speriod = m / d;
	float lperiod = pp->flags & FlagHalfPeriod ? 0.5 : 1.0;
	float loffset = pp->flags & FlagReversedHalfPeriod ? -1.0 : 1.0;

	for (unsigned int iy = 0; iy < h; iy++) {
		for (unsigned int ix = 0; ix < w; ix++) {
			float x = (float)(ix - pp->x) + v;
			float y = (float)(iy - pp->y) + v;
			float d = sqrt(x * x + y * y);
			float t = d * speriod / 16.0;
			t = t - floor(t);
			pdata->lut_dis[iy * w + ix] = round(t * 255.0);
		}
	}

	for (unsigned int ix = 0; ix < SIN_LUT_SIZE; ix++) {
		float t = (float)ix / 255.0;
		float v = (loffset * cos(M_TWOPI * lperiod * t) + 1.0) / 2.0;
		pdata->lut_sin[ix] = round(v * 255.0);
	}
}

static void init(layer_obj_t *pparam, layer_obj_t *pdata)
{
	// Allocate param buffer
	pparam->size = sizeof(param_t);
	logic_layers_alloc(pparam);
}

static void config(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int *ok,
		   unsigned int w, unsigned int h)
{
	// Allocate data buffer
	pdata->size = SIN_LUT_SIZE + w * h;
	logic_layers_alloc(pdata);
	// Check buffer valid
	if (pparam->size == 0 || pdata->size == 0) {
		pparam->size = 0;
		pdata->size = 0;
		*ok = 0;
		return;
	}
	// Generate LUTs
	updateLut(pparam->p, pdata->p, w, h);
}

static void proc(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int tick,
		 uint8_t *pfb, unsigned int w, unsigned int h)
{
	if (pparam->size == 0 || pdata->size == 0)
		return;

	param_t *pp = pparam->p;
	data_t *plut = pdata->p;

	tick = tick * pp->time.mult / pp->time.div;
	unsigned int t = (tick % 1000) * (SIN_LUT_SIZE - 1) / 999;

	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			unsigned int b = plut->lut_dis[y * w + x];
			uint8_t *pv = pfb + y * w + x;
			*pv = plut->lut_sin[(b - t) % SIN_LUT_SIZE];
		}
	}
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdSine,
	.init = &init,
	.config = &config,
	.proc = &proc,
};
