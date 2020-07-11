// Layer: Constant
// Parameters:
//   [0]	Flags (unused)
//   [1]	Value

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

typedef struct PACKED {
	uint8_t flags;
	uint8_t value;
} param_t;

static void init(layer_obj_t *pparam, layer_obj_t *pdata)
{
	// Allocate param buffer
	pparam->size = sizeof(param_t);
	logic_layers_alloc(pparam);
	pdata->size = 0;
}

static void proc(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int tick,
		 uint8_t *pfb, unsigned int w, unsigned int h)
{
	if (pparam->size == 0)
		return;

	param_t *pp = pparam->p;
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			uint8_t *pv = pfb + y * w + x;
			unsigned int a = *pv;
			const unsigned int b = 255;
			*pv = pp->value;
		}
	}
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdConst,
	.init = &init,
	.proc = &proc,
};
