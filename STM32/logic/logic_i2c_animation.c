#include <debug.h>
#include <system/systick.h>
#include <peripheral/i2c_slave.h>
#include "logic_animation.h"

#define FUNC_BASE	0x20
#define FUNC_SIZE	0x10

typedef enum {
	// Animation functions
	FuncEnable = 0
} func_t;

static uint8_t regs[FUNC_SIZE];

#if 1
static void init()
{
	regs[FuncEnable] = 1;
	logic_animation_enable(1);
}

INIT_HANDLER() = &init;
#endif

static void *i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return 0;

	func_t func = id - FUNC_BASE;
	// Segment 0 (last): Registers
	*segment = 0;
	*size = FUNC_SIZE - func;
	return &regs[func];
}

static void i2c_write(unsigned int id, unsigned int segment, unsigned int size, void *p)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return;

	logic_animation_enable(regs[FuncEnable]);
}

I2C_SLAVE_REG_HANDLER() = {&i2c_data, &i2c_write};
