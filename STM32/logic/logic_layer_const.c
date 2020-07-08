// Layer: Constant
// Parameters:
//   [0]	Flags
//     0x80	Multiply(1) or override(0)
//   [1]	Value

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

typedef struct PACKED {
	uint8_t flags;
	uint8_t v;
} param_t;

static void proc(unsigned int tick, void *param, void *ptr)
{
	param_t *pp = param;
	unsigned int w, h;
	uint8_t *p = matrix_fb(0, &w, &h);
	unsigned int v = pp->v;
	unsigned int mult = pp->flags & 0x80;
	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			uint8_t *pv = p + y * w + x;
			*pv = mult ? v * *pv / 255 : v;
		}
	}
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdConst,
	.config = 0,
	.proc = &proc,
};
