#ifndef LOGIC_LAYERS_H
#define LOGIC_LAYERS_H

#include <stdint.h>

typedef enum {LayerIdNone = 0, LayerIdConst, LayerIdGamma,
	      LayerIdBreath, LayerIdSine,
	      LayerIdString,
} layer_id_t;

typedef const struct {
	layer_id_t id;
	void *(* const config)(void *param, unsigned int *ok);
	void *(* const data)(void *param, void *ptr, unsigned int *size);
	void (* const proc)(unsigned int tick, void *param, void *ptr);
} logic_layer_handler_t;

#define LOGIC_LAYER_HANDLER()	LIST_ITEM(logic_layer, logic_layer_handler_t)

void logic_layers_enable(unsigned int e);
unsigned int logic_layers_refresh_cnt();

unsigned int logic_layers_max();
unsigned int logic_layers_param_size();

void logic_layers_select(const uint8_t *layers, unsigned int num);
void logic_layers_set_param(unsigned int layer, const void *p, unsigned int size);
void *logic_layers_param(unsigned int layer);
void *logic_layers_data(unsigned int layer, unsigned int *size);
unsigned int logic_layers_update();

void *logic_layers_alloc(unsigned int size);

#endif // LOGIC_LAYERS_H
