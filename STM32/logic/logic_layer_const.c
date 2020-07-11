// Layer: Constant
// Parameters:
//   [0]	Flags (unused)
//   [1]	Multiply factor of original value
//   [2]	Multiply factor of constant 1

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

typedef struct PACKED {
	uint8_t flags;
	uint8_t m[2];
} param_t;

static void init(layer_obj_t *pparam, layer_obj_t *pdata)
{
	// Allocate param buffer
	pparam->size = sizeof(param_t);
	logic_layers_alloc(pparam);
	pdata->size = 0;
}

static void proc(unsigned int tick, void *param, void *ptr)
{
	param_t *pp = param;
	unsigned int w, h;
	uint8_t *p = matrix_fb(0, &w, &h);
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			uint8_t *pv = p + y * w + x;
			unsigned int a = *pv;
			const unsigned int b = 255;
			*pv = ((a * pp->m[0]) + (b * pp->m[1])) / 255;
		}
	}
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdConst,
	.init = &init,
	.proc = &proc,
};
