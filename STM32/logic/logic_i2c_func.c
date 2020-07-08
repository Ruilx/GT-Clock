#include <debug.h>
#include <system/systick.h>
#include <peripheral/i2c_slave.h>
#include <peripheral/matrix.h>

#define FUNC_BASE	0xf0
#define FUNC_SIZE	0x10

typedef enum {
	// Extra
	// FuncDebug
	//   Show FPS prints if DEBUG is enabled
	FuncDebug = 0xf,
} func_t;

static uint8_t regs[FUNC_SIZE];

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
#if DEBUG > 5
	uint8_t *ptr = p;
	printf(ESC_WRITE "%lu\tlogic misc: write 0x%02x complete, segment %u, %u bytes" ESC_DATA,
	       systick_cnt(), id, segment, size);
	for (unsigned int i = 0; i < data.buf.pos; i++)
		printf(" 0x%02x", ptr[i]);
	printf("\n");
#endif
}

I2C_SLAVE_REG_HANDLER() = {&i2c_data, &i2c_write};
