#include <stdio.h>
#include <debug.h>
#include <system/systick.h>
#include <peripheral/i2c_slave.h>
#include <peripheral/matrix.h>

static uint8_t regs[256];

typedef enum {FuncX = 0x10, FuncY = 0x11, FuncPtr = 0x12} func_t;

static void *logic_i2c_fb(unsigned int *size)
{
	unsigned int w = 0, h = 0;
	unsigned int x = regs[FuncX], y = regs[FuncY];
	uint8_t *p = matrix_fb(&w, &h);
	unsigned int ofs = y * w + x;
	p += ofs;
	*size = ofs >= w * h ? 0 : w * h - ofs;
	return p;
}

static void *logic_i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	func_t func = id;
	switch (func) {
	case FuncX:
	case FuncY:
		if (*segment == 0) {
			// Segment 0: X, Y
			*segment = 1;
			*size = func == FuncX ? 2 : 1;
			return &regs[id];
		}
		// fall through
	case FuncPtr:
		// Segment 1: Matrix data
		*segment = 0;
		return logic_i2c_fb(size);
	default:
		*segment = 0;
		*size = 1;
		return &regs[id];
	}
}

static void logic_i2c_write_complete(unsigned int id, unsigned int segment, unsigned int size, void *p)
{
#if DEBUG > 5
	uint8_t *ptr = p;
	printf(ESC_WRITE "%lu\ti2c reg: write 0x%02x complete, segment %u, %u bytes" ESC_DATA,
	       systick_cnt(), id, segment, size);
	for (unsigned int i = 0; i < data.buf.pos; i++)
		printf(" 0x%02x", ptr[i]);
	printf("\n");
#endif
}

I2C_SLAVE_REG_HANDLER() = {&logic_i2c_data, &logic_i2c_write_complete};
