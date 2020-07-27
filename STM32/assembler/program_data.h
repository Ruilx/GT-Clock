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
	OP_JUMP_NOT_ZERO(LabelEnable, DataEnable),
	OP_LABEL(LabelExit),
	OP_EXIT(),

	OP_LABEL(LabelEnable),
	OP_LOAD_PARAM(DataTmp, LayerTextScroll, ParamTextFlag),
	OP_AND(DataTmp, 0x01),
	OP_JUMP_NOT_ZERO(LabelExit, DataTmp),
	OP_SAVE_PARAM(DataStart, LayerTextScroll, ParamTextFlag),
	OP_EXIT(),
};
