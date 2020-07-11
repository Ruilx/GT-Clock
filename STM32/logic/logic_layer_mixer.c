// Layer mixer
// Parameters:
//   [0]	Number of mixer steps
//   [1]	Layer x offset
//   [2]	Layer y offset
//   [3]	Layer width
//   [4]	Layer height
//   [5...]	Mixer opcode and parameters

#include <string.h>
#include <macros.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include "logic_layer_mixer.h"

#define FB_SIZE	(8 * 40)

typedef enum {OpMult, OpMultSrc, OpMultDst, OpMin, OpMax,
	      OpNegSrc, OpNegDst, OpThresSrc, OpThresDst} opcode_t;

typedef struct PACKED {
	uint8_t op;
	uint8_t param;
} op_t;

typedef struct PACKED {
	uint8_t x, y, w, h;
	op_t op[];
} param_t;

static struct {
	uint8_t fb[FB_SIZE];
} data;

void logic_layer_mixer_init(layer_obj_t *pmixer, uint8_t nops)
{
	// Allocate mixer parameter buffer
	pmixer->size = sizeof(param_t) + nops * sizeof(op_t);
	logic_layers_alloc(pmixer);
	if (pmixer->size == 0)
		return;

	param_t *pp = pmixer->p;
	unsigned int w, h;
	matrix_fb(0, &w, &h);
	pp->x = 0;
	pp->y = 0;
	pp->w = w;
	pp->h = h;
	if (w * h > FB_SIZE)
		panic();
}

void *logic_layer_mixer_fb(layer_obj_t *pmixer, unsigned int *w, unsigned int *h)
{
	if (pmixer->size == 0)
		return matrix_fb(0, w, h);
	param_t *pp = pmixer->p;
	*w = pp->w;
	*h = pp->h;
	return data.fb;
}

static inline uint8_t min(uint8_t a, uint8_t b)
{
	return a >= b ? b : a;
}

static inline uint8_t max(uint8_t a, uint8_t b)
{
	return a >= b ? a : b;
}

static void op(op_t op, uint8_t *dst, uint8_t *src, unsigned int size)
{
	switch (op.op) {
	case OpThresDst:
		for (; size--; src++, dst++)
			*dst = *dst >= op.param ? 255 : 0;
		break;
	case OpMult:
		for (; size--; src++, dst++)
			*dst = ((unsigned int)*src * *dst * op.param) / (255 * 255);
		break;
	case OpMultSrc:
		for (; size--; src++, dst++)
			*src = ((unsigned int)*src * op.param) / 255;
		break;
	case OpMultDst:
		for (; size--; src++, dst++)
			*dst = ((unsigned int)*dst * op.param) / 255;
		break;
	case OpMin:
		for (; size--; src++, dst++)
			*dst = ((unsigned int)min(*dst, *src) * op.param) / 255;
		break;
	case OpMax:
		for (; size--; src++, dst++)
			*dst = ((unsigned int)max(*dst, *src) * op.param) / 255;
		break;
	case OpNegSrc:
		for (; size--; src++, dst++)
			*src = ((unsigned int)(255 - *src) * op.param) / 255;
		break;
	case OpNegDst:
		for (; size--; src++, dst++)
			*dst = ((unsigned int)(255 - *dst) * op.param) / 255;
		break;
	}
}

void logic_layer_mixer_post(layer_obj_t *pmixer)
{
	if (pmixer->size == 0)
		return;
	unsigned int w, h;
	uint8_t *pfb = matrix_fb(0, &w, &h);
	param_t *pp = pmixer->p;
	unsigned int steps = (pmixer->size - sizeof(param_t)) / sizeof(op_t);
	for (unsigned int y = 0; y < pp->h; y++) {
		void *dst = &pfb[(pp->y + y) * w + pp->x];
		void *src = &data.fb[y * pp->w];
		if (steps == 0) {
			memcpy(dst, src, pp->w);
		} else {
			for (unsigned int s = 0; s < steps; s++)
				op(pp->op[s], dst, src, pp->w);
		}
	}
}
