#ifndef LOGIC_LAYER_MIXER_H
#define LOGIC_LAYER_MIXER_H

#include <stdint.h>
#include "logic_layers.h"

void logic_layer_mixer_init(layer_obj_t *pmixer, uint8_t ops);
void *logic_layer_mixer_fb(layer_obj_t *pmixer, unsigned int *w, unsigned int *h);
void logic_layer_mixer_post(layer_obj_t *pmixer);

#endif // LOGIC_LAYER_MIXER_H
