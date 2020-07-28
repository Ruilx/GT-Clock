#pragma once

#include <logic_layer_program_data.h>

enum {OpLabel = 0xff};

#define OP_EXIT()			{0, OpExit, {}}
#define OP_JUMP(addr)			{1, OpJump, {addr}}
#define OP_JUMP_ZERO(addr)		{1, OpJumpZero, {addr}}
#define OP_JUMP_NOT_ZERO(addr)		{1, OpJumpNotZero, {addr}}
#define OP_JUMP_NEGATIVE(addr)		{1, OpJumpNegative, {addr}}

#define OP_PTR_MIXER(layer, addr)	{2, OpPtrMixer, {layer, addr}}
#define OP_PTR_PARAM(layer, addr)	{2, OpPtrParam, {layer, addr}}
#define OP_PTR_DATA(layer, addr)	{2, OpPtrData, {layer, addr}}
#define OP_PTR_CALENDAR(addr)		{1, OpPtrCalendar, {addr}}

#define OP_PRINT_I8(type)		{1, OpPrintI8, {type}}

#define OP_LOAD_CONST(val)		{1, OpLoadConst, {val}}
#define OP_LOAD_DATA(src)		{1, OpLoadData, {src}}
#define OP_SAVE_DATA(dst)		{1, OpSaveData, {dst}}

#define OP_LOAD_PTR()			{0, OpLoadPtr, {}}
#define OP_SAVE_PTR()			{0, OpSavePtr, {}}

#define OP_AND(mask)			{1, OpAnd, {mask}}
#define OP_OR(mask)			{1, OpOr, {mask}}
#define OP_XOR(mask)			{1, OpXor, {mask}}

#define OP_INC()			{0, OpInc, {}}
#define OP_DEC()			{0, OpDec, {}}

// Special
#define OP_LABEL(label)			{1, OpLabel, {label}}
