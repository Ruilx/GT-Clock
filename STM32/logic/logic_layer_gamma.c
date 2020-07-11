// Layer: Gamma
// Parameters:
//   [0]	Flags (unused)
//   [1]	Factor in hex: 0xAB A.B

#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

typedef struct PACKED {
	uint8_t flags;
	uint8_t factor;
} param_t;

static void updateLut(float factor, uint8_t *ptr)
{
	float max = powf(255, factor);
	for (unsigned int i = 0; i < 256; i++)
		*(ptr + i) = round(255.0 * powf(i, factor) / max);
}

static void init(layer_obj_t *pparam, layer_obj_t *pdata)
{
	// Allocate param and data buffers
	pparam->size = sizeof(param_t);
	logic_layers_alloc(pparam);
	pdata->size = 256;
	logic_layers_alloc(pdata);
	if (pparam->size == 0 || pdata->size == 0) {
		pparam->size = 0;
		pdata->size = 0;
	}
}

static void config(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int *ok)
{
	if (pparam->size == 0 || pdata->size == 0) {
		*ok = 0;
		return;
	}

	param_t *pp = pparam->p;
	uint8_t ifactor = pp->factor;
	float factor = ((ifactor >> 4) & 0xf) + (ifactor & 0xf) / 10;
	updateLut(factor, pdata->p);
}

static void proc(unsigned int tick, void *param, void *ptr)
{
	uint8_t *lut = ptr;
	unsigned int w, h;
	uint8_t *p = matrix_fb(0, &w, &h);
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			uint8_t *pv = p + y * w + x;
			*pv = lut[*pv];
		}
	}
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdGamma,
	.init = &init,
	.config = &config,
	.proc = &proc,
};
