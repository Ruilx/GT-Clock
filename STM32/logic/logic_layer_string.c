// Layer: String
// Parameters:
//   [0]	Flags (unused)
//   [1]	X top left offset
//   [2]	Y top left offset

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>
#include <fonts.h>

static const FontInfo fonts[] = {
	font_ruilxfixedsys,
};

typedef struct PACKED {
	uint8_t flags;
	uint8_t m[2];
} param_t;

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
	.id = LayerIdString,
	.config = 0,
	.proc = &proc,
};
