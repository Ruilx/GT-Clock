#include <string.h>
#include <debug.h>
#include <system/irq.h>
#include <system/systick.h>
#include <peripheral/matrix.h>
#include "logic_layers.h"
#include "logic_layer_mixer.h"
#include "logic_layer_program.h"

#define MAX_LAYERS	16
#define HEAP_SIZE	(6 * 1024)
#define LAYER_OBJS	3

#define DEBUG_HEAP	5

enum {Active = 0, Inactive};

typedef struct {
	const logic_layer_handler_t *phdr;
	layer_obj_t obj[LObjs];
} layer_t;

typedef enum {PCode = 0, PData, PObjs} program_obj_enum_t;
typedef struct {
	layer_obj_t obj[PObjs];
} program_t;

static struct {
	unsigned int refcnt;
	unsigned int enable;
	volatile unsigned int commit;
	layer_t layer[MAX_LAYERS][2];
	program_t program[2];
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

static void layer_reset(unsigned int layer)
{
	layer_t *pp = &data.layer[layer][Inactive];
	pp->phdr = 0;
	for (unsigned int iobj = 0; iobj < LObjs; iobj++)
		pp->obj[iobj].size = 0;
}

static inline void heap_gc()
{
	// Find active objects
	layer_obj_t *objlist[2 * (MAX_LAYERS * LObjs + PObjs)];
	unsigned int objnum = 0;
	for (unsigned int iact = 0; iact < 2; iact++) {
		for (unsigned int ilayer = 0; ilayer < MAX_LAYERS; ilayer++) {
			if (&data.layer[ilayer][iact].phdr == 0)
				continue;
			for (unsigned int iobj = 0; iobj < LObjs; iobj++) {
				layer_obj_t *pp = &data.layer[ilayer][iact].obj[iobj];
				if (pp->size != 0)
					objlist[objnum++] = pp;
			}
		}
		for (unsigned int iobj = 0; iobj < PObjs; iobj++) {
			layer_obj_t *pp = &data.program[iact].obj[iobj];
			if (pp->size != 0)
				objlist[objnum++] = pp;
		}
	}

	void *p = data.heap.buf;
	for (;;) {
next:		; // Find next object in heap space
		void *pnext = data.heap.buf + data.heap.size;
		unsigned int size = 0;
		for (unsigned int iobj = 0; iobj < objnum; iobj++) {
			layer_obj_t *pp = objlist[iobj];
			if (pp->size == 0)
				continue;
			if (pp->p >= p && pp->p < pnext) {
				if (pp->p == p) {
					p += pp->size;
					goto next;
				}
				pnext = pp->p;
				size = pp->size;
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
			for (unsigned int iobj = 0; iobj < objnum; iobj++) {
				layer_obj_t *pp = objlist[iobj];
				if (pp->p == pnext)
					pp->p = p;
			}
		}
		p += size;
	}

	unsigned int size = data.heap.size;
	unsigned int newsize = p - (void *)data.heap.buf;
	if (size == newsize)
		return;
#if DEBUG > 5
	printf(ESC_DEBUG "%lu\tlayers: GC complete: "
			 "%u bytes total, %u bytes in use, %u bytes freed\n",
	       systick_cnt(), HEAP_SIZE, newsize, size - newsize);
#endif
	data.heap.size = newsize;
}

#if DEBUG >= DEBUG_HEAP
static void heap_debug()
{
	static size_t psize = 0;
	size_t hsize = data.heap.size;
	if (psize == hsize)
		return;
	psize = hsize;
	printf(ESC_DEBUG "%lu\tlayers: %u bytes allocated\n", systick_cnt(), hsize);
}

IDLE_HANDLER() = &heap_debug;
#endif

void CAN1_SCE_IRQHandler()
{
	// Software rendering thread
	// Which also handles GC
	if (!data.enable)
		goto gc;

	uint32_t tick = systick_cnt();
	for (unsigned int i = 0; i < MAX_LAYERS; i++) {
		layer_t *pp = &data.layer[i][Active];
		if (pp->phdr == 0)
			continue;
		unsigned int w, h;
		void *pfb = logic_layer_mixer_fb(&pp->obj[LOMixer], &w, &h);
		pp->phdr->proc(&pp->obj[LOParam], &pp->obj[LOData], tick, pfb, w, h);
		logic_layer_mixer_post(&pp->obj[LOMixer]);
	}

	matrix_fb_swap();
	data.refcnt++;

	// Process program code
	if (data.program[Active].obj[PCode].size != 0)
		logic_layer_program_run(&data.program[Active].obj[PCode],
					&data.program[Active].obj[PData]);

gc:	// Now, GC
	if (data.commit) {
		if (data.commit == 0xff) {
			for (unsigned int i = 0; i < MAX_LAYERS; i++)
				data.layer[i][Active] = data.layer[i][Inactive];
			data.program[Active] = data.program[Inactive];
		}
		heap_gc();
		data.commit = 0;
	}
	NVIC_ClearPendingIRQ(CAN1_SCE_IRQn);
}

static void trigger()
{
	// Trigger rendering thread if enabled
	if (data.enable)
		NVIC_SetPendingIRQ(CAN1_SCE_IRQn);
}

MATRIX_SWAP_HANDLER() = &trigger;

void logic_layers_select(const uint8_t *layers, int8_t start, unsigned int num)
{
	unsigned int i;
	if (start < 0) {
		for (i = 0; i < MAX_LAYERS && i <= (unsigned int)(MAX_LAYERS + start); i++)
			layer_reset(i);
		start = 0;
	}
	for (i = start; i < MAX_LAYERS && i < start + num; i++) {
		layer_reset(i);
		if (layers[i - start] == LayerIdNone)
			continue;
		layer_t *pp = &data.layer[i][Inactive];
		LIST_ITERATE(logic_layer, logic_layer_handler_t, phdr) {
			if (phdr->id == layers[i - start]) {
				pp->phdr = phdr;
				if (phdr->init)
					phdr->init(&pp->obj[LOParam], &pp->obj[LOData]);
				break;
			}
		}
	}
}

void *logic_layers_active_obj(unsigned int layer, layer_obj_enum_t obj, unsigned int *size)
{
	layer_t *pp = &data.layer[layer][Active];
	*size = pp->obj[obj].size;
	return *size == 0 ? 0 : pp->obj[obj].p;
}

void *logic_layers_inactive_obj(unsigned int layer, layer_obj_enum_t obj, unsigned int *size)
{
	layer_t *pp = &data.layer[layer][Inactive];
	*size = pp->obj[obj].size;
	return *size == 0 ? 0 : pp->obj[obj].p;
}

void *logic_layers_param(unsigned int layer, unsigned int *size)
{
	return logic_layers_inactive_obj(layer, LOParam, size);
}

void *logic_layers_mixer(unsigned int layer, unsigned int nops, unsigned int *size)
{
	layer_t *pp = &data.layer[layer][Inactive];
	if (pp->obj[LOMixer].size == 0)
		logic_layer_mixer_init(&pp->obj[LOMixer], nops);
	return logic_layers_inactive_obj(layer, LOMixer, size);
}

void *logic_layers_data(unsigned int layer, unsigned int *size)
{
	return logic_layers_inactive_obj(layer, LOData, size);
}

static void *logic_layers_program_obj(program_obj_enum_t obj, unsigned int *size)
{
	layer_obj_t *pd = &data.program[Inactive].obj[obj];
	*size = pd->size;
	return pd->p;
}

void *logic_layers_program(unsigned int progsize, unsigned int paramsize, unsigned int *size)
{
	layer_obj_t *pp = &data.program[Inactive].obj[PCode];
	pp->size = progsize;
	logic_layers_alloc(pp);
	layer_obj_t *pd = &data.program[Inactive].obj[PData];
	pd->size = paramsize;
	logic_layers_alloc(pd);

	if (pp->size == 0 || pd->size != paramsize) {
		pp->size = 0;
		pd->size = 0;
	}
	logic_layer_program_init(pp, pd);
	return logic_layers_program_obj(PCode, size);
}

void *logic_layers_program_code(unsigned int *size)
{
	return logic_layers_program_obj(PCode, size);
}

void *logic_layers_program_data(unsigned int *size)
{
	return logic_layers_program_obj(PData, size);
}

unsigned int logic_layers_commit(unsigned int layer)
{
	layer_t *pp = &data.layer[layer][Inactive];
	if (pp->phdr == 0) {
		layer_reset(layer);
		return 1;
	}
	unsigned int w, h;
	logic_layer_mixer_fb(&pp->obj[LOMixer], &w, &h);
	unsigned int ok = 1;
	if (pp->phdr->config)
		pp->phdr->config(&pp->obj[LOParam], &pp->obj[LOData], &ok, w, h);
	//gc(1);
	return ok;
}

void logic_layers_gc(unsigned int commit)
{
	data.commit = commit;
	// Trigger GC immediately if rendering is disabled
	if (!data.enable)
		NVIC_SetPendingIRQ(CAN1_SCE_IRQn);
	while (data.commit);
}

void logic_layers_alloc(layer_obj_t *obj)
{
	unsigned int size = obj->size;
	if (data.heap.size + size > HEAP_SIZE) {
#if DEBUG
		printf(ESC_ERROR "%lu\tlayers: No space left: "
				 "%u bytes in use, %u bytes requested\n",
		       systick_cnt(), data.heap.size, size);
#endif
		obj->size = 0;
		return;
	}
#if DEBUG > 5
	printf(ESC_DEBUG "%lu\tlayers: Allocate buffer: "
			 "%u bytes total, %u bytes in use, %u bytes requested\n",
	       systick_cnt(), HEAP_SIZE, data.heap.size, size);
#endif
	obj->p = &data.heap.buf[data.heap.size];
	data.heap.size += size;
}

#if DEBUG > 5
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
