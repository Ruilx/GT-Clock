#include <stdio.h>
#include <debug.h>
#include <system/systick.h>
#include <peripheral/i2c_slave.h>
#include <peripheral/matrix.h>

typedef enum {
	// Framebuffer functions 0x10 - 0x1f
	// FuncSwap
	//   Write 0 to access active FB
	//   Write 1 to access standby FB
	//   Write 0x80 to copy active FB to standby FB
	//   Write 0xff to activate standby FB
	// FuncX, FuncY
	//   Starting offset of FuncPtr
	FuncSwap = 0x10, FuncX = 0x1d, FuncY = 0x1e, FuncPtr = 0x1f,
	// Extra
	// FuncDebug
	//   Show FPS prints if DEBUG is enabled
	FuncDebug = 0xff,
} func_t;

static uint8_t regs[256];

#if DEBUG
static void debug()
{
	static uint32_t v = 0;
	if (!regs[FuncDebug]) {
		v = systick_cnt();
		return;
	}

	if ((systick_cnt() - v) >= 1000) {
		// Matrix refresh
		static unsigned int matrix_cnt = 0;
		unsigned int matrix_cnt_now = matrix_refresh_cnt();
		unsigned int matrix_cnt_delta = matrix_cnt_now - matrix_cnt;
		matrix_cnt = matrix_cnt_now;

		printf(ESC_DEBUG "%lu\tdebug: FPS: Matrix %u\n",
		       systick_cnt(), matrix_cnt_delta);
		v += 1000;
	}
}

IDLE_HANDLER(&debug);
#endif

static void *logic_i2c_fb(unsigned int *size)
{
	unsigned int w = 0, h = 0;
	unsigned int x = regs[FuncX], y = regs[FuncY];
	uint8_t *p = matrix_fb(!!regs[FuncSwap], &w, &h);
	unsigned int ofs = y * w + x;
	p += ofs;
	*size = ofs >= w * h ? 0 : w * h - ofs;
	return p;
}

static void *logic_i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	switch ((func_t)id) {
	case FuncSwap:
	case FuncX:
	case FuncY:
		if (*segment == 0) {
			// Segment 0: X, Y
			*segment = 1;
			*size = FuncPtr - id;
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

	// Skip if no data was written
	if (segment == 1 && size == 0)
		return;

	switch ((func_t)id) {
	case FuncSwap:
		if (regs[FuncSwap] == 0xff)
			matrix_fb_swap();
		else if (regs[FuncSwap] == 0x80)
			matrix_fb_copy();
		break;
	default:
		break;
	}
}

I2C_SLAVE_REG_HANDLER() = {&logic_i2c_data, &logic_i2c_write_complete};
