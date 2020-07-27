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
#if DEBUG >= 6
	uint8_t (*ppc)[32] = pcode->p;
	uint8_t (*ppd)[32] = pdata->p;
#endif

	// Temporary register and pointer
	uint8_t reg = 0;
	uint8_t *ptr = 0;

	// Program execution
	uint8_t *pc = pcode->p;
	uint8_t *pd = pdata->p;
	unsigned int ofs = 0;
	for (;;) {
		switch ((opcode_t)pc[ofs]) {
		case OpExit:
			return;
		case OpJump:
			ofs = pc[ofs + 1];
			break;
		case OpJumpNotZero:
			ofs = reg != 0 ? pc[ofs + 1] : ofs + 2;
			break;
		case OpJumpNegative:
			ofs = (int8_t)reg < 0 ? pc[ofs + 1] : ofs + 2;
			break;

		case OpPtrMixer:
		case OpPtrParam:
		case OpPtrData: {
			layer_obj_enum_t obj;
			switch ((opcode_t)pc[ofs]) {
			case OpPtrMixer:
				obj = LOMixer;
				break;
			case OpPtrParam:
				obj = LOParam;
				break;
			case OpPtrData:
				obj = LOData;
				break;
			default:
				break;
			}
			unsigned int size;
			ptr = logic_layers_active_obj(pc[ofs + 1], obj, &size);
			if (ptr == 0 || pc[ofs + 2] >= size)
				ptr = 0;
			else
				ptr += pc[ofs + 2];
			ofs += 3;
			break;
		}

		case OpLoadConst:
			reg = pc[ofs + 1];
			ofs += 2;
			break;
		case OpLoadData:
			if (pc[ofs + 1] < pdata->size)
				reg = pd[pc[ofs + 1]];
			ofs += 2;
			break;

		case OpLoadPtr:
			if (ptr != 0)
				reg = *ptr;
			ofs += 1;
			break;
		case OpSavePtr:
			if (ptr != 0)
				*ptr = reg;
			ofs += 1;
			break;

		case OpAnd:
			reg &= pc[ofs + 1];
			ofs += 2;
			break;
		case OpOr:
			reg |= pc[ofs + 1];
			ofs += 2;
			break;
		case OpXor:
			reg ^= pc[ofs + 2];
			ofs += 2;
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
