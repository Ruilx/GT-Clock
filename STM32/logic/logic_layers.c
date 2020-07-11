#include <string.h>
#include <debug.h>
#include <system/irq.h>
#include <system/systick.h>
#include <peripheral/matrix.h>
#include "logic_layers.h"
#include "logic_layer_mixer.h"

#define MAX_LAYERS	16
#define HEAP_SIZE	(4 * 1024)
#define LAYER_OBJS	3

typedef enum {LParam = 0, LData, LMixer, LObjs} obj_t;

typedef struct {
	const logic_layer_handler_t *phdr;
	layer_obj_t obj[LObjs];
} layer_t;

static struct {
	unsigned int enable;
	unsigned int refcnt;
	volatile unsigned int commit;
	volatile unsigned int actparam;
	layer_t layer[MAX_LAYERS][2];
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
		unsigned int obj = 0;
		// Find next object in heap space
		for (unsigned int ilayer = 0; ilayer < MAX_LAYERS; ilayer++) {
			for (unsigned int iact = 0; iact < 2; iact++) {
				for (unsigned int iobj = 0; iobj < LObjs; iobj++) {
					layer_obj_t *pp = &data.layer[ilayer][iact].obj[iobj];
					if (pp->size == 0)
						continue;
					if (pp->p >= p && pp->p < pnext) {
						pnext = pp->p;
						size = pp->size;
						layer = ilayer;
						obj = iobj;
					}
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
			for (unsigned int iact = 0; iact < 2; iact++) {
				layer_obj_t *pp = &data.layer[layer][iact].obj[obj];
				if (pp->size != 0 && pp->p == pnext)
					pp->p = p;
			}
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
		layer_t *pp = &data.layer[i][actparam];
		if (pp->phdr == 0)
			continue;
		unsigned int w, h;
		void *pfb = logic_layer_mixer_fb(&pp->obj[LMixer], &w, &h);
		pp->phdr->proc(&pp->obj[LParam], &pp->obj[LData], tick, pfb, w, h);
		logic_layer_mixer_post(&pp->obj[LMixer]);
	}

	matrix_fb_swap();
	data.refcnt++;

gc:
	if (data.commit) {
		if (data.commit == 0xff) {
			unsigned int actparam = data.actparam;
			for (unsigned int i = 0; i < MAX_LAYERS; i++)
				data.layer[i][actparam] = data.layer[i][!actparam];
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

void logic_layers_select(const uint8_t *layers, unsigned int start, unsigned int num)
{
	unsigned int actparam = data.actparam;
	unsigned int i;
	for (i = start; i < MAX_LAYERS && i < start + num; i++) {
		layer_t *pp = &data.layer[i][!actparam];
		pp->phdr = 0;
		pp->obj[LParam].size = 0;
		pp->obj[LData].size = 0;
		pp->obj[LMixer].size = 0;
		if (layers[i - start] == LayerIdNone)
			continue;
		LIST_ITERATE(logic_layer, logic_layer_handler_t, phdr) {
			if (phdr->id == layers[i - start]) {
				pp->phdr = phdr;
				phdr->init(&pp->obj[LParam], &pp->obj[LData]);
			}
		}
	}
}

static void *logic_layers_obj(unsigned int layer, obj_t obj, unsigned int *size)
{
	unsigned int actparam = data.actparam;
	layer_t *pp = &data.layer[layer][!actparam];
	*size = pp->obj[obj].size;
	return *size == 0 ? 0 : pp->obj[obj].p;
}

void *logic_layers_param(unsigned int layer, unsigned int *size)
{
	return logic_layers_obj(layer, LParam, size);
}

void *logic_layers_mixer(unsigned int layer, unsigned int nops, unsigned int *size)
{
	unsigned int actparam = data.actparam;
	layer_t *pp = &data.layer[layer][!actparam];
	if (pp->obj[LMixer].size == 0)
		logic_layer_mixer_init(&pp->obj[LMixer], nops);
	return logic_layers_obj(layer, LMixer, size);
}

void *logic_layers_data(unsigned int layer, unsigned int *size)
{
	return logic_layers_obj(layer, LData, size);
}

unsigned int logic_layers_commit(unsigned int layer)
{
	unsigned int actparam = data.actparam;
	layer_t *pp = &data.layer[layer][!actparam];
	if (pp->phdr == 0) {
		pp->obj[LParam].size = 0;
		pp->obj[LData].size = 0;
		pp->obj[LMixer].size = 0;
		return 1;
	}
	unsigned int w, h;
	logic_layer_mixer_fb(&pp->obj[LMixer], &w, &h);
	unsigned int ok = 1;
	if (pp->phdr->config)
		pp->phdr->config(&pp->obj[LParam], &pp->obj[LData], &ok, w, h);
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
		uint8_t mix[16];
		const void *pdata;
	} data[MAX_LAYERS] = {
		{LayerIdSine, {
			0x81,	// Multiply, half-aligned
			20,	// X offset
			0,	// Y offset
			1,	// Spacial period, multiply
			3,	// Spacial period, divide
			1,	// Temporal period, multiply
			2,	// Temporal period, divide
		}, {0xff}, 0},
		{LayerIdConst, {
			0x00,	// Flags
			255,	// Value
		}, {2, 25, 1, 14, 5,	6, 255, 0, 120}, 0},
		{LayerIdString, {
			0x00,	// Flags
			-3,	// X offset
			0,	// Y offset
			5,	// String length
			37,	// Font ID
		}, {2, 17, 3, 20, 4,	2, 120, 4, 255}, "STM32"},
		{LayerIdGamma, {
			0x00,	// Flags
			0x22,	// Factor in hex
		}, {0xff}, 0},
	};

	uint8_t layers[MAX_LAYERS];
	for (unsigned int layer = 0; layer < MAX_LAYERS; layer++)
		layers[layer] = data[layer].id;
	logic_layers_select(layers, 0, MAX_LAYERS);
	for (unsigned int layer = 0; layer < MAX_LAYERS; layer++) {
		if (data[layer].id == LayerIdNone)
			continue;
		unsigned int size = 0;
		void *p = logic_layers_param(layer, &size);
		if (size != 0)
			memcpy(p, data[layer].param, size);
		if (!logic_layers_commit(layer))
			dbgbkpt();
		if (data[layer].mix[0] != 0xff) {
			p = logic_layers_mixer(layer, data[layer].mix[0], &size);
			if (size != 0)
				memcpy(p, &data[layer].mix[1], size);
			if (!logic_layers_commit(layer))
				dbgbkpt();
		}
		p = logic_layers_data(layer, &size);
		if (size != 0 && data[layer].pdata != 0)
			memcpy(p, data[layer].pdata, size);
	}
	logic_layers_update();
	logic_layers_enable(1);
}

INIT_HANDLER() = &init_test;
#endif
