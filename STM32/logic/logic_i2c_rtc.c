#include <debug.h>
#include <system/systick.h>
#include <peripheral/rtc.h>
#include <peripheral/i2c_slave.h>
#include <logic/logic_layers.h>

#define FUNC_BASE	0x50
#define FUNC_SIZE	0x10

typedef enum {
	// RTC time function
	// FuncTime
	//   Read or set RTC time
	//   Data: Second (u8), Minute (u8), Hour (u8)
	//         Day (u8), Month (u8), Year (u16)
	FuncTime = 0,
	// FuncTimestamp
	//   Read or set RTC time in u64 timestamp format
	FuncTimestamp,
	NumFuncs
} func_t;

static uint8_t regs[NumFuncs];

typedef struct PACKED {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t mday;
	uint8_t mon;
	uint16_t year;
} tm_t;

static struct {
	struct {
		union {
			tm_t tm;
			uint8_t data[8];
			uint64_t ts;
		};
	} time;
} data;

static void rtc_update_tm()
{
	struct tm *ptm = rtc_time();
	data.time.tm = (tm_t){
		.sec  = ptm->tm_sec,
		.min  = ptm->tm_min,
		.hour = ptm->tm_hour,
		.mday = ptm->tm_mday,
		.mon  = ptm->tm_mon,
		.year = ptm->tm_year,
	};
}

static void rtc_set_tm()
{
	struct tm dtm = (struct tm) {
		.tm_sec  = data.time.tm.sec,
		.tm_min  = data.time.tm.min,
		.tm_hour = data.time.tm.hour,
		.tm_mday = data.time.tm.mday,
		.tm_mon  = data.time.tm.mon,
		.tm_year  = data.time.tm.year,
	};
	rtc_set_time(&dtm);
}

static void *i2c_data(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size)
{
	if (id < FUNC_BASE || id >= (FUNC_BASE + FUNC_SIZE))
		return 0;

	func_t func = id - FUNC_BASE;
	switch (func) {
	case FuncTime:
		// Update RTC time when reading
		if (!write)
			rtc_update_tm();
		*size = sizeof(data.time.tm);
		return &data.time.tm;
	case FuncTimestamp:
		if (!write)
			data.time.ts = rtc_timestamp();
		*size = sizeof(data.time.ts);
		return &data.time.ts;
	default:
		break;
	}

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
	case FuncTime:
		if (size == sizeof(data.time.tm))
			rtc_set_tm();
		break;
	default:
		break;
	}
}

I2C_SLAVE_REG_HANDLER() = {.data = &i2c_data, .write_complete = &i2c_write};
