#pragma once

#include <stdint.h>
#include "program_inc.h"

enum {LayerSine, LayerSineDisabled, LayerTextConst, LayerTextScroll, LayerGamma};
enum {ParamTextFlag};

enum {DataEnable, DataStart, DataTmp, SizeData};
enum label_t {LabelEnable, LabelExit};

struct op_t {
	uint8_t size;
	uint8_t op;
	uint8_t data[4];
};

static const op_t program_code[] = {
	OP_LOAD_DATA(DataEnable),
	OP_JUMP_NOT_ZERO(LabelEnable),
	OP_EXIT(),

	OP_LABEL(LabelEnable),
	OP_PTR_PARAM(LayerTextScroll, ParamTextFlag),
	OP_LOAD_PTR(),
	OP_AND(0x01),
	OP_JUMP_NOT_ZERO(LabelExit),
	OP_LOAD_DATA(DataStart),
	OP_SAVE_PTR(),

	OP_LABEL(LabelExit),
};
