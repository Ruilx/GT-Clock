// Layer program processor

#include <string.h>
#include <macros.h>
#include <debug.h>
#include <peripheral/rtc.h>
#include "logic_layer_program.h"
#include "logic_layer_program_data.h"

#define DEBUG_CHECK	5

typedef struct PACKED {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t mday;
	uint8_t mon;
	uint8_t wday;
	uint8_t yday;
	uint8_t dst;
	uint16_t year;
} tm_t;

static struct {
	struct {
		union {
			tm_t tm;
			uint8_t data[10];
		};
	} time[2];
	uint8_t tidx;
} data;

// Synchronise time struct
static void time_sync()
{
	struct tm *ptm = rtc_time();
	unsigned int idx = !data.tidx;
	data.time[idx].tm = (tm_t){
		.sec  = ptm->tm_sec,
		.min  = ptm->tm_min,
		.hour = ptm->tm_hour,
		.mday = ptm->tm_mday,
		.mon  = ptm->tm_mon,
		.wday = ptm->tm_wday,
		.yday = ptm->tm_yday,
		.dst  = ptm->tm_isdst,
		.year = ptm->tm_year,
	};
	data.tidx = idx;
}

RTC_SECOND_HANDLER() = &time_sync;

static inline void print_i8(print_type_t type, char *ptr, unsigned int size, int8_t data)
{
	if (data >= 100) {
		if (size == 0)
			return;
		size--;
		*ptr++ = '0' + data / 100;
		data = data % 100;
	}
	if (data >= 10 || type == PType02d) {
		if (size == 0)
			return;
		size--;
		*ptr++ = '0' + data / 10;
		data = data % 10;
	}
	if (size == 0)
		return;
	size--;
	*ptr++ = '0' + data;
}

void logic_layer_program_init(layer_obj_t *pcode, layer_obj_t *pdata)
{
	if (pcode->size != 0)
		*(uint8_t *)pcode->p = OpExit;
	if (pdata->size != 0)
		memset(pdata->p, 0, pdata->size);
}

void logic_layer_program_run(layer_obj_t *pcode, layer_obj_t *pdata)
{
#if DEBUG >= 5
	uint8_t (*ppc)[32] = pcode->p;
	uint8_t (*ppd)[32] = pdata->p;
#endif

	// Temporary register and pointer
	uint8_t reg = 0;
	uint8_t *ptr = 0;
	unsigned int size = 0;

	// Program execution
	uint8_t *pc = pcode->p;
	uint8_t *pd = pdata->p;
	unsigned int ofs = 0;
	for (;;) {
		switch ((opcode_t)pc[ofs]) {
		case OpExit:
			return;
		case OpJump:
			ofs = ofs + (int8_t)pc[ofs + 1];
			break;
		case OpJumpZero:
			ofs = reg == 0 ? ofs + (int8_t)pc[ofs + 1] : ofs + 2;
			break;
		case OpJumpNotZero:
			ofs = reg != 0 ? ofs + (int8_t)pc[ofs + 1] : ofs + 2;
			break;
		case OpJumpNegative:
			ofs = (int8_t)reg < 0 ? ofs + (int8_t)pc[ofs + 1] : ofs + 2;
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
			ptr = logic_layers_active_obj(pc[ofs + 1], obj, &size);
			if (ptr == 0 || pc[ofs + 2] >= size)
				ptr = 0;
			else
				ptr += pc[ofs + 2];
			ofs += 3;
			break;
		}
		case OpPtrCalendar:
			if (pc[ofs + 1] < sizeof(data.time[0].data))
				ptr = &data.time[data.tidx].data[pc[ofs + 1]];
			else
				ptr = 0;
			ofs += 2;
			break;

		case OpPrintI8:
			print_i8(pc[ofs + 1], (char *)ptr, size, reg);
			ofs += 2;
			break;

		case OpLoadConst:
			reg = pc[ofs + 1];
			ofs += 2;
			break;
		case OpLoadData:
			if (pc[ofs + 1] < pdata->size)
				reg = pd[pc[ofs + 1]];
			ofs += 2;
			break;
		case OpSaveData:
			if (pc[ofs + 1] < pdata->size)
				pd[pc[ofs + 1]] = reg;
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

		case OpInc:
			reg++;
			ofs += 1;
			break;
		case OpDec:
			reg--;
			ofs += 1;
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
