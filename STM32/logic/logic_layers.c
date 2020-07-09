#include <string.h>
#include <debug.h>
#include <system/systick.h>
#include <peripheral/matrix.h>
#include "logic_layers.h"

#define MAX_LAYERS	16
#define PARAM_SIZE	12
#define HEAP_SIZE	(4 * 1024)

typedef struct {
	const logic_layer_handler_t *phdr;
	uint8_t param[PARAM_SIZE];
	void *p;
} layer_t;

static struct {
	unsigned int enable;
	unsigned int refcnt;
	layer_t layer[MAX_LAYERS];
	struct {
		unsigned int size;
		uint8_t buf[HEAP_SIZE];
	} heap;
} data;

LIST(logic_layer, logic_layer_handler_t);

void logic_layers_enable(unsigned int e)
{
	data.enable = !!e;
}

unsigned int logic_layers_refresh_cnt()
{
	return data.refcnt;
}

static void proc()
{
	static unsigned int _tick = 0;
	uint32_t tick = systick_cnt();
	if (_tick == tick)
		return;

	_tick = tick;
	if (!data.enable)
		return;
	if (!matrix_fb_ready())
		return;

	for (unsigned int i = 0; i < MAX_LAYERS; i++) {
		if (data.layer[i].phdr == 0)
			break;
		data.layer[i].phdr->proc(tick, data.layer[i].param, data.layer[i].p);
	}

	matrix_fb_swap();
	data.refcnt++;
}

IDLE_HANDLER() = &proc;

void logic_layers_select(const uint8_t *layers)
{
	unsigned int i;
	for (i = 0; i < MAX_LAYERS; i++) {
		data.layer[i].phdr = 0;
		LIST_ITERATE(logic_layer, logic_layer_handler_t, phdr)
			if (phdr->id == layers[i])
				data.layer[i].phdr = phdr;
	}
	if (i != MAX_LAYERS)
		data.layer[i].phdr = 0;
}

void logic_layers_set_param(unsigned int layer, const void *p, unsigned int size)
{
	if (size > PARAM_SIZE)
		size = PARAM_SIZE;
	memcpy(data.layer[layer].param, p, size);
}

unsigned int logic_layers_update()
{
	unsigned int ok = 1;
	data.heap.size = 0;
	for (unsigned int i = 0; i < MAX_LAYERS; i++)
		if (data.layer[i].phdr && data.layer[i].phdr->config)
			data.layer[i].p = data.layer[i].phdr->config(data.layer[i].param, PARAM_SIZE, &ok);
	return ok;
}

void *logic_layers_alloc(unsigned int size)
{
	if (data.heap.size + size > HEAP_SIZE) {
		printf(ESC_ERROR "%lu\tlayers: No buffer space left: "
				 "%u bytes total, %u bytes used, %u bytes requested\n",
		       systick_cnt(), HEAP_SIZE, data.heap.size, size);
		return 0;
	}
#if DEBUG
	printf(ESC_DEBUG "%lu\tlayers: Allocating buffer: "
			 "%u bytes total, %u bytes used, %u bytes requested\n",
	       systick_cnt(), HEAP_SIZE, data.heap.size, size);
#endif
	void *p = &data.heap.buf[data.heap.size];
	data.heap.size += size;
	return p;
}

#if 1
static void init()
{
	unsigned int w, h;
	matrix_fb(0, &w, &h);

	static const uint8_t layers[MAX_LAYERS] = {
		LayerIdSine,
		LayerIdSine,
		LayerIdConst,
		LayerIdGamma,
		LayerIdNone
	};
	const uint8_t params[MAX_LAYERS][PARAM_SIZE] = {
		{
			0x80,	// Multiply, not half-aligned
			w - 1,	// X offset
			0,	// Y offset
			0,	// Factor of original value
			255,	// Factor of new value
			0,	// Factor of mixer value
			1,	// Spacial period, multiply
			2,	// Spacial period, divide
			1,	// Temporal period, multiply
			4,	// Temporal period, divide
		}, {
			0x81,	// Multiply, half-aligned
			24,	// X offset
			4,	// Y offset
			70,	// Factor of original value
			20,	// Factor of new value
			165,	// Factor of mixer value
			5,	// Spacial period, multiply
			2,	// Spacial period, divide
			1,	// Temporal period, multiply
			2,	// Temporal period, divide
		}, {
			0x00,	// Flags
			210,	// Factor of original value
			0,	// Factor of constant 1
		}, {
			0x00,	// Flags
			0x22,	// Factor in hex
		},
	};

	logic_layers_select(layers);
	for (unsigned int layer = 0; layer < MAX_LAYERS; layer++)
		logic_layers_set_param(layer, params[layer], PARAM_SIZE);
	logic_layers_update();
}

INIT_HANDLER() = &init;
#endif
