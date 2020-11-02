#include <debug.h>
#include <system/systick.h>
#include <peripheral/i2c_slave.h>
#include "logic_layers.h"

#define FUNC_BASE	0x20
#define FUNC_SIZE	0x10

#define BUF_SIZE	16

typedef enum {
	// Enable animations
	FuncEnable = 0,
	// Configure layers
	FuncLayers,
	// Access layer mixer data
	FuncMixer,
	// Access layer parameters
	FuncParam,
	// Access layer private data
	FuncData,
	// Initialise program code
	FuncProgramInit = 5,
	// Access program code
	FuncProgramCode,
	// Access program code, addressed
	FuncProgramCodeAddr,
	// Access program data
	FuncProgramData,
	// Access program data, addressed
	FuncProgramDataAddr,
	// 0xff: Trigger layer updates
	// 0x01: Trigger garbage collection
	FuncUpdate = 0xf,
} func_t;

static struct {
	uint8_t regs[FUNC_SIZE];
	uint8_t buf[BUF_SIZE];
} data;

static void *i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return 0;

	uint8_t *p;
	func_t func = id - FUNC_BASE;
	switch (func) {
	case FuncEnable:
	case FuncUpdate:
		// Register access
		*segment = 0;
		*size = 1;
		return &data.regs[func];
	case FuncLayers:
		switch (*segment) {
		case 0:
			// Register access, starting layer
			*segment = 1;
			*size = 1;
			return &data.regs[func];
		case 1:
			// Layer buffer
			*segment = 0;
			*size = logic_layers_max();
			return data.buf;
		}
		break;
	case FuncMixer:
		switch (*segment) {
		case 0:
			// Register access, select layer
			*segment = 1;
			*size = 1;
			return &data.regs[FuncParam];
		case 1:
			// Register access, number of mixer ops
			*segment = 2;
			*size = 1;
			return &data.regs[FuncMixer];
		case 2:
			// Layer mixer data
			*segment = 3;
			return logic_layers_mixer(data.regs[FuncParam], data.regs[FuncMixer], size);
		case 3:
			// Layer parameters
			*segment = 4;
			return logic_layers_param(data.regs[FuncParam], size);
		case 4:
			// Layer private data
			*segment = 0;
			data.regs[FuncUpdate] = logic_layers_commit(data.regs[FuncParam]);
			return logic_layers_data(data.regs[FuncParam], size);
		}
		break;
	case FuncParam:
		switch (*segment) {
		case 0:
			// Layer parameters
			*segment = 1;
			return logic_layers_param(data.regs[FuncParam], size);
		case 1:
			// Layer private data
			*segment = 0;
			data.regs[FuncUpdate] = logic_layers_commit(data.regs[FuncParam]);
			return logic_layers_data(data.regs[FuncParam], size);
		}
		break;
	case FuncData:
		// Layer private data
		*segment = 0;
		return logic_layers_data(data.regs[FuncParam], size);
	case FuncProgramInit:
		switch (*segment) {
		case 0:
			// Register access, program size and data size
			*segment = 1;
			*size = 2;
			return &data.regs[func];
		case 1:
			// Program code
			*segment = 0;
			return logic_layers_program(data.regs[FuncProgramInit],
						    data.regs[FuncProgramInit + 1], size);
		}
		break;
	case FuncProgramCode:
		// Program code
		*segment = 0;
		return logic_layers_program_code(size);
	case FuncProgramCodeAddr:
		switch (*segment) {
		case 0:
			// Register access, program code offset
			*segment = 1;
			*size = 1;
			return &data.regs[func];
		case 1:
			// Program code
			*segment = 0;
			p = logic_layers_program_code(size);
			if (data.regs[FuncProgramCodeAddr] >= *size) {
				*size = 0;
				return 0;
			}
			*size = *size - data.regs[FuncProgramCodeAddr];
			return p + data.regs[FuncProgramCodeAddr];
		}
		break;
	case FuncProgramData:
		// Program data
		*segment = 0;
		return logic_layers_program_data(size);
	case FuncProgramDataAddr:
		switch (*segment) {
		case 0:
			// Register access, program data offset
			*segment = 1;
			*size = 1;
			return &data.regs[func];
		case 1:
			// Program data
			*segment = 0;
			p = logic_layers_program_data(size);
			if (data.regs[FuncProgramDataAddr] >= *size) {
				*size = 0;
				return 0;
			}
			*size = *size - data.regs[FuncProgramDataAddr];
			return p + data.regs[FuncProgramDataAddr];
		}
		break;
	default:
		break;
	}
	// Invalid functions
	*segment = 0;
	*size = 0;
	return 0;
}

static void i2c_write(unsigned int id, unsigned int segment, unsigned int size, void *p)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return;

	func_t func = id - FUNC_BASE;
	switch (func) {
	case FuncEnable:
		if (size > 0)
			logic_layers_enable(data.regs[FuncEnable]);
		break;
	case FuncLayers:
		if (size >= 0 && segment == 0)
			logic_layers_select(data.buf, (int8_t)data.regs[FuncLayers], size);
		break;
	case FuncMixer:
		if (segment == 4)
			data.regs[FuncUpdate] = logic_layers_commit(data.regs[FuncParam]);
		break;
	case FuncParam:
		if (segment == 1)
			data.regs[FuncUpdate] = logic_layers_commit(data.regs[FuncParam]);
		break;
	case FuncProgramInit:
		if ((segment == 1 && size == 2) || (segment == 0 && size == 0))
			logic_layers_program(data.regs[FuncProgramInit],
					     data.regs[FuncProgramInit + 1], &size);
		break;
	case FuncUpdate:
		if (size > 0)
			logic_layers_gc(data.regs[FuncUpdate]);
	default:
		break;
	}
}

I2C_SLAVE_REG_HANDLER() = {&i2c_data, &i2c_write};
