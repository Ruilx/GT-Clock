#include <debug.h>
#include <system/systick.h>
#include <peripheral/rtc.h>
#include <peripheral/i2c_slave.h>
// For debug prints
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>

#define FUNC_BASE	0xf0
#define FUNC_SIZE	0x10

#define DEBUG_TEST	5
#define DEBUG_FPS	1

typedef enum {
	// General function
	// FuncUID
	//   Report chip UID and flash size
	FuncUID = 0,
#if DEBUG >= DEBUG_TEST
	// FuncTest
	//   Test I2C communication
	FuncTestNum = 0xc,
	FuncTestRep = 0xd,
	FuncTestVal = 0xe,
#endif
#if DEBUG >= DEBUG_FPS
	// FuncDebug
	//   Show FPS prints if DEBUG is enabled
	FuncDebug = 0xf,
#endif
	FuncReserved,
} func_t;

static uint8_t regs[FUNC_SIZE];

static struct {
#if DEBUG >= DEBUG_TEST
	uint8_t num, rep;
#endif
} data;

#if DEBUG > 5
static void init()
{
	regs[FuncDebug] = 1;
}

INIT_HANDLER() = &init;
#endif

#if DEBUG >= DEBUG_FPS
static void debug()
{
	if (!regs[FuncDebug])
		return;

	// RTC value
	uint32_t rtc = rtc_timestamp();

	// Matrix refresh
	static unsigned int matrix_cnt = 0;
	unsigned int matrix_cnt_now = matrix_refresh_cnt();
	unsigned int matrix_cnt_delta = matrix_cnt_now - matrix_cnt;
	matrix_cnt = matrix_cnt_now;

	// Layers refresh
	static unsigned int layers_cnt = 0;
	unsigned int layers_cnt_now = logic_layers_refresh_cnt();
	unsigned int layers_cnt_delta = layers_cnt_now - layers_cnt;
	layers_cnt = layers_cnt_now;

	printf(ESC_DEBUG "%lu\tdebug cnt: RTC %lu, Matrix %u, Layers %u\n", systick_cnt(),
	       rtc, matrix_cnt_delta, layers_cnt_delta);
}

RTC_SECOND_HANDLER() = &debug;
#endif

static void *i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return 0;

	func_t func = id - FUNC_BASE;
	switch (func) {
	case FuncUID:
		if (*segment == 0) {
			// UID
			*segment = write ? 0 : 1;
			*size = write ? 0 : 12;
			return (void *)UID_BASE;
		} else {
			// Flash size
			*segment = 0;
			*size = 2;
			return (void *)FLASHSIZE_BASE;
		}
		break;
#if DEBUG >= DEBUG_TEST
	case FuncTestNum:
		*size = 1;
		if (*segment == 0) {
			*segment = 1;
			if (write)
				regs[FuncTestRep] = 1;
			return &regs[func];
		} else {
			*segment = 0;
			return &regs[FuncTestRep];
		}
	case FuncTestRep:
		*segment = 0;
		*size = 1;
		return &regs[func];
	case FuncTestVal:
		if (write && *segment != 0 && regs[func] != *segment - 1)
			dbgbkpt();
		(*segment)++;
		regs[func] = *segment;
		return &regs[func];
#endif
#if DEBUG >= DEBUG_FPS
	case FuncDebug:
		*segment = 0;
		*size = 1;
		return &regs[func];
#endif
	default:
		break;
	}

	*segment = 0;
	*size = 0;
	return 0;
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
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return;

	func_t func = id - FUNC_BASE;
	switch (func) {
#if DEBUG >= DEBUG_TEST
	case FuncTestNum:
		if (regs[func] != 0xff && (data.num != regs[FuncTestNum] || data.rep != regs[FuncTestRep]))
			dbgbkpt();
		data.rep = 0;
		break;
	case FuncTestVal:
		if (segment != 0 && regs[func] != segment - 1)
			dbgbkpt();
		if (data.rep != 0 && data.num != segment)
			dbgbkpt();
		data.num = segment;
		data.rep++;
		break;
#endif
	default:
		break;
	}
}

I2C_SLAVE_REG_HANDLER() = {.data = &i2c_data, .write_complete = &i2c_write};
