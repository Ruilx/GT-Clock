#pragma once

#include <logic_layer_program_data.h>

enum {OpLabel = 0xff};

#define OP_EXIT()			{0, OpExit, {}}
#define OP_JUMP(addr)			{1, OpJump, {addr}}
#define OP_JUMP_NOT_ZERO(addr, src)	{2, OpJumpNotZero, {addr, src}}
#define OP_JUMP_NEGATIVE(addr, src)	{2, OpJumpNegative, {addr, src}}

#define OP_LOAD_MIXER(dst, layer, addr)	{3, OpLoadMixer, {dst, layer, addr}}
#define OP_LOAD_PARAM(dst, layer, addr)	{3, OpLoadParam, {dst, layer, addr}}
#define OP_LOAD_DATA(dst, layer, addr)	{3, OpLoadData, {dst, layer, addr}}
#define OP_LOAD_CONST(dst, val)		{2, OpLoadConst, {dst, val}}

#define OP_SAVE_MIXER(src, layer, addr)	{3, OpSaveMixer, {src, layer, addr}}
#define OP_SAVE_PARAM(src, layer, addr)	{3, OpSaveParam, {src, layer, addr}}
#define OP_SAVE_DATA(src, layer, addr)	{3, OpSaveData, {src, layer, addr}}

#define OP_AND(dst, mask)		{2, OpAnd, {dst, mask}}
#define OP_OR(dst, mask)		{2, OpAnd, {dst, mask}}
#define OP_XOR(dst, mask)		{2, OpAnd, {dst, mask}}

// Special
#define OP_LABEL(label)			{1, OpLabel, {label}}
