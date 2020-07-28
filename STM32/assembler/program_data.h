#pragma once

#include <stdint.h>
#include "program_inc.h"

struct op_t {
	uint8_t size;
	uint8_t op;
	uint8_t data[4];
};

#if 1

enum {LayerSine, LayerTextTime, LayerGamma};
enum {ParamTextFlag};
enum {MixerX, MixerY, MixerW, MixerH, MixerOp1, MixerOp1Param, MixerOp2, MixerOp2Param};

enum {DataEnable, SizeData};
enum label_t {LabelExit};

static const op_t program_code[] = {
	// Check program enabled
	OP_LOAD_DATA(DataEnable),
	OP_JUMP_ZERO(LabelExit),

	// Update RTC time
	OP_PTR_CALENDAR(2),	// Hour
	OP_LOAD_PTR(),
	OP_PTR_DATA(LayerTextTime, 0),
	OP_PRINT_I8(PType02d),

	OP_PTR_CALENDAR(1),	// Minute
	OP_LOAD_PTR(),
	OP_PTR_DATA(LayerTextTime, 2),
	OP_PRINT_I8(PType02d),

	OP_PTR_CALENDAR(0),	// Second
	OP_LOAD_PTR(),
	OP_PTR_DATA(LayerTextTime, 4),
	OP_PRINT_I8(PType02d),

	// Exit
	OP_LABEL(LabelExit),
};

#else

enum {LayerSine, LayerSineDisabled, LayerTextConst, LayerTextScroll, LayerGamma};
enum {ParamTextFlag};
enum {MixerX, MixerY, MixerW, MixerH, MixerOp1, MixerOp1Param, MixerOp2, MixerOp2Param};

enum {DataEnable, DataStart, DataState, SizeData};
enum label_t {LabelDec, LabelScroll, LabelWait, LabelInc,
	      LabelDecDone, LabelIncDone,
	      LabelExit};

static const op_t program_code[] = {
	// Check program enabled
	OP_LOAD_DATA(DataEnable),
	OP_JUMP_ZERO(LabelExit),
	OP_LOAD_DATA(DataState),

	// State: Decrease intensity
	OP_LABEL(LabelDec),
	OP_JUMP_NOT_ZERO(LabelScroll),

	// Check done condition
	OP_PTR_MIXER(LayerTextConst, MixerOp1Param),
	OP_LOAD_PTR(),
	OP_JUMP_ZERO(LabelDecDone),
	// Decrease TextConst layer mixer value by 1
	OP_DEC(),
	OP_SAVE_PTR(),
	OP_EXIT(),
	// Done, jump to next state
	OP_LABEL(LabelDecDone),
	OP_LOAD_CONST(LabelScroll - LabelDec),
	OP_SAVE_DATA(DataState),
	OP_EXIT(),

	// State: Start scrolling
	OP_LABEL(LabelScroll),
	OP_DEC(),
	OP_JUMP_NOT_ZERO(LabelWait),

	// Trigger scrolling
	OP_PTR_PARAM(LayerTextScroll, ParamTextFlag),
	OP_LOAD_DATA(DataStart),
	OP_SAVE_PTR(),
	// Jump to next state
	OP_LOAD_CONST(LabelWait - LabelDec),
	OP_SAVE_DATA(DataState),
	OP_EXIT(),

	// State: Wait for scrolling
	OP_LABEL(LabelWait),
	OP_DEC(),
	OP_JUMP_NOT_ZERO(LabelInc),

	// Check done condition
	OP_PTR_PARAM(LayerTextScroll, ParamTextFlag),
	OP_LOAD_PTR(),
	OP_AND(0x01),
	OP_JUMP_NOT_ZERO(LabelExit),
	// Jump to next state
	OP_LOAD_CONST(LabelInc - LabelDec),
	OP_SAVE_DATA(DataState),
	OP_EXIT(),

	// State: Increase intensity
	OP_LABEL(LabelInc),

	// Increase TextConst layer mixer value by 1
	OP_PTR_MIXER(LayerTextConst, MixerOp1Param),
	OP_LOAD_PTR(),
	OP_INC(),
	// Check done condition
	OP_JUMP_ZERO(LabelIncDone),
	OP_SAVE_PTR(),
	OP_EXIT(),
	// Done, reset to first state
	OP_LABEL(LabelIncDone),
	OP_LOAD_CONST(0),
	OP_SAVE_DATA(DataState),
	// Disable program if not auto restart
	OP_LOAD_DATA(DataEnable),
	OP_DEC(),
	OP_JUMP_NOT_ZERO(LabelExit),
	OP_SAVE_DATA(DataEnable),

	// Exit
	OP_LABEL(LabelExit),
};

#endif
