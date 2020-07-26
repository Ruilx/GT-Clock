// Layer: Bitmap
// Parameters:
//   [0]	Flags (unused)

#include <string.h>
#include "logic_layers.h"

#pragma pack(push,1)

typedef struct {
	uint8_t flags;
} param_t;

#pragma pack(pop)

static void init(layer_obj_t *pparam, layer_obj_t *pdata)
{
	// Allocate param buffer, data buffer need to wait until commit
	pparam->size = sizeof(param_t);
	logic_layers_alloc(pparam);
	pdata->size = 0;
}

static void config(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int *ok,
		   unsigned int w, unsigned int h)
{
	if (pparam->size == 0) {
		*ok = 0;
		return;
	}

	// Now, allocate data buffer
	unsigned int bsize = w * h;
	if (pdata->size != bsize) {
		pdata->size = bsize;
		logic_layers_alloc(pdata);
		if (pdata->size == 0) {
			*ok = 0;
			return;
		}
	}
}

static void proc(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int tick,
		 uint8_t *pfb, unsigned int w, unsigned int h)
{
	if (pparam->size == 0 || pdata->size == 0)
		return;
	memcpy(pfb, pdata->p, pdata->size);
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdBitmap,
	.init = &init,
	.config = &config,
	.proc = &proc,
};
