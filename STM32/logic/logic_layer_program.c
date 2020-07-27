// Layer program processor

#include <string.h>
#include <macros.h>
#include <debug.h>
#include "logic_layer_program.h"
#include "logic_layer_program_data.h"

#define DEBUG_CHECK	5

void logic_layer_program_init(layer_obj_t *pcode, layer_obj_t *pdata)
{
	if (pcode->size == 0)
		return;
	*(uint8_t *)pcode->p = OpExit;
}

void logic_layer_program_run(layer_obj_t *pcode, layer_obj_t *pdata)
{
#if DEBUG >= 5
	uint8_t (*ppc)[32] = pcode->p;
	uint8_t (*ppd)[32] = pdata->p;
#endif
	uint8_t *pc = pcode->p;
	uint8_t *pd = pdata->p;
	uint8_t *p;
	unsigned int size;
	unsigned int ofs = 0;
	for (;;) {
		switch ((opcode_t)pc[ofs]) {
		case OpExit:
			return;
		case OpJump:
			ofs = pc[ofs + 1];
			break;
		case OpJumpNotZero:
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 2] >= pdata->size)
				dbgbkpt();
#endif
			ofs = pd[pc[ofs + 2]] != 0 ? pc[ofs + 1] : ofs + 3;
			break;
		case OpJumpNegative:
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 2] >= pdata->size)
				dbgbkpt();
#endif
			ofs = (int8_t)pd[pc[ofs + 2]] < 0 ? pc[ofs + 1] : ofs + 3;
			break;
		case OpLoadMixer:
			p = logic_layers_mixer(pc[ofs + 2], 0, &size);
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 3] >= size || pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] = p[pc[ofs + 3]];
			ofs += 4;
			break;
		case OpLoadParam:
			p = logic_layers_param(pc[ofs + 2], &size);
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 3] >= size || pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] = p[pc[ofs + 3]];
			ofs += 4;
			break;
		case OpLoadData:
			p = logic_layers_data(pc[ofs + 2], &size);
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 3] >= size || pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] = p[pc[ofs + 3]];
			ofs += 4;
			break;
		case OpLoadConst:
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] = pc[ofs + 2];
			ofs += 3;
			break;
		case OpSaveMixer:
			p = logic_layers_mixer(pc[ofs + 2], 0, &size);
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 3] >= size || pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			p[pc[ofs + 3]] = pd[pc[ofs + 1]];
			ofs += 4;
			break;
		case OpSaveParam:
			p = logic_layers_param(pc[ofs + 2], &size);
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 3] >= size || pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			p[pc[ofs + 3]] = pd[pc[ofs + 1]];
			ofs += 4;
			break;
		case OpSaveData:
			p = logic_layers_data(pc[ofs + 2], &size);
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 3] >= size || pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			p[pc[ofs + 3]] = pd[pc[ofs + 1]];
			ofs += 4;
			break;
		case OpAnd:
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] &= pc[ofs + 2];
			ofs += 3;
			break;
		case OpOr:
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] |= pc[ofs + 2];
			ofs += 3;
			break;
		case OpXor:
#if DEBUG >= DEBUG_CHECK
			if (pc[ofs + 1] >= pdata->size)
				dbgbkpt();
#endif
			pd[pc[ofs + 1]] ^= pc[ofs + 2];
			ofs += 3;
			break;
		case NumOps:
			break;
		}
		if (ofs == pcode->size)
			return;
#if DEBUG >= DEBUG_CHECK
		else if (ofs > pcode->size)
			dbgbkpt();
#endif
	}
}
