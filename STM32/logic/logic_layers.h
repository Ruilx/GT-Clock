#ifndef LOGIC_LAYERS_H
#define LOGIC_LAYERS_H

#include <stdint.h>
#include <list.h>

typedef enum {LayerIdNone = 0, LayerIdConst, LayerIdGamma,
	      LayerIdBreath, LayerIdSine,
	      LayerIdString, LayerIdBitmap,
} layer_id_t;

typedef struct {
	void *p;
	unsigned int size;
} layer_obj_t;

typedef const struct {
	layer_id_t id;
	void (* const init)(layer_obj_t *pparam, layer_obj_t *pdata);
	void (* const config)(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int *ok,
			      unsigned int w, unsigned int h);
	void (* const proc)(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int tick,
			    uint8_t *pfb, unsigned int w, unsigned int h);
} logic_layer_handler_t;

#define LOGIC_LAYER_HANDLER()	LIST_ITEM(logic_layer, logic_layer_handler_t)

void logic_layers_enable(unsigned int e);
unsigned int logic_layers_refresh_cnt();

unsigned int logic_layers_max();

void logic_layers_select(const uint8_t *layers, unsigned int start, unsigned int num);
void logic_layers_set_param(unsigned int layer, const void *p, unsigned int size);
void *logic_layers_param(unsigned int layer, unsigned int *size);
void *logic_layers_mixer(unsigned int layer, unsigned int nops, unsigned int *size);
void *logic_layers_data(unsigned int layer, unsigned int *size);
void *logic_layers_program(unsigned int progsize, unsigned int paramsize, unsigned int *size);
void *logic_layers_program_code(unsigned int *size);
void *logic_layers_program_data(unsigned int *size);

unsigned int logic_layers_commit(unsigned int layer);
void logic_layers_gc(unsigned int commit);

static inline unsigned int logic_layers_update()
{
	// Commit all changes and GC
	logic_layers_gc(0xff);
	return 1;
}

void logic_layers_alloc(layer_obj_t *obj);

#endif // LOGIC_LAYERS_H
