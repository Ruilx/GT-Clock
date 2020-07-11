#include <string.h>
#include <debug.h>
#include <system/irq.h>
#include <system/systick.h>
#include <peripheral/matrix.h>
#include "logic_layers.h"

#define MAX_LAYERS	16
#define HEAP_SIZE	(4 * 1024)

typedef struct {
	const logic_layer_handler_t *phdr;
	layer_obj_t param, data;
} param_t;

static struct {
	unsigned int enable;
	unsigned int refcnt;
	volatile unsigned int commit;
	volatile unsigned int actparam;
	struct {
		param_t param[2];
	} layer[MAX_LAYERS];
	struct {
		volatile unsigned int size;
		uint8_t buf[HEAP_SIZE];
	} heap;
} data;

LIST(logic_layer, logic_layer_handler_t);

static void init()
{
	// Use CAN1_SCE_IRQHandler as rendering thread
	uint32_t pg = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(CAN1_SCE_IRQn, NVIC_EncodePriority(pg, NVIC_PRIORITY_RENDERING, 0));
	NVIC_EnableIRQ(CAN1_SCE_IRQn);
}

INIT_HANDLER() = &init;

void logic_layers_enable(unsigned int e)
{
	data.enable = !!e;
}

unsigned int logic_layers_refresh_cnt()
{
	return data.refcnt;
}

unsigned int logic_layers_max()
{
	return MAX_LAYERS;
}

static inline void heap_gc()
{
	// TODO GC here
	void *p = data.heap.buf;
	for (;;) {
		void *pnext = data.heap.buf + data.heap.size;
		unsigned int size = 0;
		unsigned int layer = 0;
		for (unsigned int i = 0; i < MAX_LAYERS; i++) {
			// Find next object in heap space
			layer_obj_t *pp[4] = {
				&data.layer[i].param[0].param,
				&data.layer[i].param[0].data,
				&data.layer[i].param[1].param,
				&data.layer[i].param[1].data,
			};
			for (unsigned int iobj = 0; iobj < 4; iobj++) {
				if (pp[iobj]->size == 0)
					continue;
				if (pp[iobj]->p >= p && pp[iobj]->p < pnext) {
					pnext = pp[iobj]->p;
					size = pp[iobj]->size;
					layer = i;
				}
			}
		}
		// Check for completion
		if (size == 0)
			break;
		// Move that object if address is not contiguous
		if (p != pnext) {
#if DEBUG > 5
			printf(ESC_DEBUG "%lu\tlayers: GC move buffer: "
					 "from %u to %u, %u bytes\n",
			       systick_cnt(), pnext - (void *)data.heap.buf, p - (void *)data.heap.buf, size);
#endif
			memmove(p, pnext, size);
			// Update pointers
			layer_obj_t *pp[4] = {
				&data.layer[layer].param[0].param,
				&data.layer[layer].param[0].data,
				&data.layer[layer].param[1].param,
				&data.layer[layer].param[1].data,
			};
			for (unsigned int iobj = 0; iobj < 4; iobj++)
				if (pp[iobj]->size != 0 && pp[iobj]->p == pnext)
					pp[iobj]->p = p;
		}
		p += size;
	}
	unsigned int size = data.heap.size;
	unsigned int newsize = p - (void *)data.heap.buf;
	if (size == newsize)
		return;
#if DEBUG > 4
	printf(ESC_DEBUG "%lu\tlayers: GC complete: "
			 "%u bytes total, %u bytes in use, %u bytes freed\n",
	       systick_cnt(), HEAP_SIZE, newsize, size - newsize);
#endif
	data.heap.size = newsize;
}

void CAN1_SCE_IRQHandler()
{
	// Software rendering thread
	// Which also handles GC
	if (!data.enable)
		goto gc;

	uint32_t tick = systick_cnt();
	unsigned int actparam = data.actparam;
	for (unsigned int i = 0; i < MAX_LAYERS; i++) {
		param_t *pp = &data.layer[i].param[actparam];
		if (pp->phdr == 0)
			continue;
		pp->phdr->proc(tick, pp->param.p, pp->data.p);
	}

	matrix_fb_swap();
	data.refcnt++;

gc:
	if (data.commit) {
		if (data.commit == 0xff) {
			unsigned int actparam = data.actparam;
			for (unsigned int i = 0; i < MAX_LAYERS; i++)
				data.layer[i].param[actparam] = data.layer[i].param[!actparam];
		}
		heap_gc();
		data.commit = 0;
	}
	NVIC_ClearPendingIRQ(CAN1_SCE_IRQn);
}

static void gc(unsigned int commit)
{
	data.commit = commit;
	// Trigger GC immediately if rendering is disabled
	if (!data.enable)
		NVIC_SetPendingIRQ(CAN1_SCE_IRQn);
	while (data.commit);
}

static void trigger(uint32_t tick)
{
	// Trigger rendering thread if vsync ready
	if (data.enable && matrix_fb_ready())
		NVIC_SetPendingIRQ(CAN1_SCE_IRQn);
}

SYSTICK_HANDLER() = &trigger;

void logic_layers_select(const uint8_t *layers, unsigned int num)
{
	unsigned int actparam = data.actparam;
	unsigned int i;
	for (i = 0; i < MAX_LAYERS && i < num; i++) {
		param_t *pp = &data.layer[i].param[!actparam];
		pp->phdr = 0;
		pp->param.size = 0;
		pp->data.size = 0;
		if (layers[i] == LayerIdNone)
			continue;
		LIST_ITERATE(logic_layer, logic_layer_handler_t, phdr) {
			if (phdr->id == layers[i]) {
				pp->phdr = phdr;
				phdr->init(&pp->param, &pp->data);
			}
		}
	}
}

void *logic_layers_param(unsigned int layer, unsigned int *size)
{
	unsigned int actparam = data.actparam;
	param_t *pp = &data.layer[layer].param[!actparam];
	*size = pp->param.size;
	return pp->param.p;
}

void *logic_layers_data(unsigned int layer, unsigned int *size)
{
	*size = 0;
	unsigned int actparam = data.actparam;
	param_t *pp = &data.layer[layer].param[!actparam];
	*size = pp->data.size;
	return pp->data.size == 0 ? 0 : pp->data.p;
}

unsigned int logic_layers_commit(unsigned int layer)
{
	unsigned int actparam = data.actparam;
	param_t *pp = &data.layer[layer].param[!actparam];
	if (pp->phdr == 0) {
		pp->param.size = 0;
		pp->data.size = 0;
		return 1;
	}
	unsigned int ok = 1;
	if (pp->phdr->config)
		pp->phdr->config(&pp->param, &pp->data, &ok);
	gc(1);
	return ok;
}

unsigned int logic_layers_update()
{
	// Commit all changes and GC
	gc(0xff);
	return 1;
}

void logic_layers_alloc(layer_obj_t *obj)
{
	unsigned int size = obj->size;
	if (data.heap.size + size > HEAP_SIZE) {
		printf(ESC_ERROR "%lu\tlayers: No heap space left: "
				 "%u bytes total, %u bytes in use, %u bytes requested\n",
		       systick_cnt(), HEAP_SIZE, data.heap.size, size);
		obj->p = 0;
		obj->size = 0;
		return;
	}
#if DEBUG > 4
	printf(ESC_DEBUG "%lu\tlayers: Allocate buffer: "
			 "%u bytes total, %u bytes in use, %u bytes requested\n",
	       systick_cnt(), HEAP_SIZE, data.heap.size, size);
#endif
	obj->p = &data.heap.buf[data.heap.size];
	data.heap.size += size;
}

#if DEBUG > 4
static void init_test()
{
	static const struct {
		uint8_t id;
		uint8_t param[16];
		void *pdata;
	} data[MAX_LAYERS] = {
		{LayerIdSine, {
			0x81,	// Multiply, half-aligned
			20,	// X offset
			0,	// Y offset
			0,	// Factor of original value
			255,	// Factor of new value
			0,	// Factor of mixer value
			5,	// Spacial period, multiply
			2,	// Spacial period, divide
			1,	// Temporal period, multiply
			2,	// Temporal period, divide
		}, 0},
		{LayerIdConst, {
			0x00,	// Flags
			150,	// Factor of original value
			0,	// Factor of constant 1
		}, 0},
		{LayerIdString, {
			0x00,	// Flags
			8,	// X offset
			1,	// Y offset
			5,	// String length
			37,	// Font ID
		}, "STM32"},
		{LayerIdGamma, {
			0x00,	// Flags
			0x22,	// Factor in hex
		}, 0},
	};

	uint8_t layers[MAX_LAYERS];
	for (unsigned int layer = 0; layer < MAX_LAYERS; layer++)
		layers[layer] = data[layer].id;
	logic_layers_select(layers, MAX_LAYERS);
	for (unsigned int layer = 0; layer < MAX_LAYERS; layer++) {
		if (data[layer].id == LayerIdNone)
			continue;
		unsigned int size = 0;
		void *p = logic_layers_param(layer, &size);
		if (size != 0)
			memcpy(p, data[layer].param, size);
		if (!logic_layers_commit(layer))
			dbgbkpt();
		p = logic_layers_data(layer, &size);
		if (size != 0 && data[layer].pdata != 0)
			memcpy(p, data[layer].pdata, size);
	}
	logic_layers_update();
	logic_layers_enable(1);
}

INIT_HANDLER() = &init_test;
#endif
