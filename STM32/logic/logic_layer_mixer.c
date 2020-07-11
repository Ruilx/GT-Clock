// Layer mixer
// Parameters:
//   [0]	Flags (unused)
//   [1]	Multiply factor of original value
//   [2]	Multiply factor of constant 1

#include <macros.h>
#include "logic_layer_mixer.h"

typedef enum {MixNone = 0} type_t;

typedef struct PACKED {
	uint8_t type;
} param_t;

void logic_layer_mixer_init(layer_obj_t *pmixer)
{
	// Allocate mixer parameter buffer
	pmixer->size = sizeof(param_t);
	logic_layers_alloc(pmixer);
	if (pmixer->size != 0)
		((param_t *)pmixer->p)->type = MixNone;
}
