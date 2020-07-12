// Layer: Breathing
// Parameters:

#include <string.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

#if 0
static void config(layer_obj_t *param, layer_obj_t *data, unsigned int *ok,
		   unsigned int w, unsigned int h)
{
}
#endif

static void proc(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int tick,
		 uint8_t *pfb, unsigned int w, unsigned int h)
{
	uint8_t v = ((tick & 0x0100) ? 0xff : 0) ^ (tick & 0xff);
	memset(pfb, v, w * h);
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdBreath,
	//.config = &config,
	.proc = &proc,
};
