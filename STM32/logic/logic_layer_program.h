#pragma once

#include <stdint.h>
#include "logic_layers.h"

void logic_layer_program_init(layer_obj_t *pcode, layer_obj_t *pdata);
void logic_layer_program_run(layer_obj_t *pcode, layer_obj_t *pdata);
