#include <debug.h>
#include <system/systick.h>
#include <peripheral/i2c_slave.h>
#include "logic_layers.h"

#define FUNC_BASE	0x20
#define FUNC_SIZE	0x10

#define BUF_SIZE	32

typedef enum {
	// Enable animations
	FuncEnable = 0,
	// Configure layers
	FuncLayers,
	// Access layer parameters
	FuncParam,
	// Access layer private data
	FuncData,
	// Trigger layer updates
	FuncUpdate,
} func_t;

static struct {
	uint8_t regs[FUNC_SIZE];
	uint8_t buf[BUF_SIZE];
} data;

#if 1
static void init()
{
	data.regs[FuncEnable] = 1;
	logic_layers_enable(1);
}

INIT_HANDLER() = &init;
#endif

static void *i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return 0;

	func_t func = id - FUNC_BASE;
	switch (func) {
	case FuncEnable:
	case FuncUpdate:
		// Register access
		*segment = 0;
		*size = 1;
		return &data.regs[func];
	case FuncLayers:
		// Layer buffer
		*segment = 0;
		*size = logic_layers_max();
		return data.buf;
	case FuncParam:
		switch (*segment) {
		case 0:
			// Register access
			*segment = 1;
			*size = 1;
			return &data.regs[func];
		case 1:
			// Layer parameters
			*segment = 0;
			*size = logic_layers_param_size();
			return logic_layers_param(data.regs[FuncParam]);
		}
		break;
	case FuncData:
		// Layer private data
		*segment = 0;
		return logic_layers_data(data.regs[FuncParam], size);
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
	if (size == 0)
		return;

	func_t func = id - FUNC_BASE;
	switch (func) {
	case FuncEnable:
		logic_layers_enable(data.regs[FuncEnable]);
		break;
	case FuncLayers:
		logic_layers_select(data.buf, size);
		break;
	case FuncUpdate:
		data.regs[FuncUpdate] = logic_layers_update();
		break;
	default:
		break;
	}
}

I2C_SLAVE_REG_HANDLER() = {&i2c_data, &i2c_write};
