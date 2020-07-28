#include <device.h>
#include <debug.h>
#include <time.h>
#include <system/systick.h>
#include "rtc.h"

#define UPDATE_PERIOD	100

struct {
	union {
		uint32_t cnt;
		struct {
			uint16_t cntl;
			uint16_t cnth;
		};
	} cnt;
	struct tm time;
} data;

void rtc_init()
{
	// Enable power and backup interface clocks
	RCC->APB1ENR |= RCC_APB1ENR_PWREN_Msk | RCC_APB1ENR_BKPEN_Msk;
	// Enable access to backup registers and RTC
	PWR->CR |= PWR_CR_DBP_Msk;
	// LSE ON
	RCC->BDCR |= RCC_BDCR_LSEON_Msk;
	// Wait for LSE ready
	while (!(RCC->BDCR | RCC_BDCR_LSERDY_Msk));
	// Check and set RTC clock source
	if ((RCC->BDCR & RCC_BDCR_RTCSEL_Msk) != RCC_BDCR_RTCSEL_LSE) {
		// Reset backup domain, keep LSE ON
		RCC->BDCR = RCC_BDCR_BDRST_Msk | RCC_BDCR_LSEON_Msk;
		// Clear reset
		RCC->BDCR = RCC_BDCR_LSEON_Msk;
		// Select LSE as RTC clock source
		RCC->BDCR = RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON_Msk;

		// Wait for RTC register ready
		while (!(RTC->CRL & RTC_CRL_RTOFF));
		// Disable RTC interrupts
		RTC->CRH = 0;
		// Enter RTC configuration mode
		RTC->CRL = RTC_CRL_CNF_Msk;
		// Set up RTC clock prescaler
		union {
			uint32_t prl;
			struct {
				uint16_t prll;
				uint16_t prlh;
			};
		} presc;
		presc.prl = 32768 - 1;		// 32k osc
		RTC->PRLH = presc.prlh;
		RTC->PRLL = presc.prll;
		// Reset RTC counter
		RTC->CNTH = 0;
		RTC->CNTL = 0;
		// Exit RTC configuration mode
		RTC->CRL = 0;
		// Wait for RTC register ready
		while (!(RTC->CRL & RTC_CRL_RTOFF));
	}
	// Enable RTC clock
	RCC->BDCR |= RCC_BDCR_RTCEN_Msk;
}

INIT_HANDLER() = &rtc_init;

static void rtc_update()
{
	// Update every UPDATE_PERIOD systick
	static unsigned int prevtick = 0;
	uint32_t tick = systick_cnt();
	if ((tick - prevtick) < UPDATE_PERIOD)
		return;
	prevtick += UPDATE_PERIOD;

	uint32_t prevcnt = data.cnt.cnt;
	for (;;) {
		uint32_t cnt;
		// Read RTC counter value
		data.cnt.cntl = RTC->CNTL;
		data.cnt.cnth = RTC->CNTH;
		cnt = data.cnt.cnt;
		// Read RTC counter value again
		data.cnt.cntl = RTC->CNTL;
		data.cnt.cnth = RTC->CNTH;
		// Make sure RTC counter did not change
		if (cnt == data.cnt.cnt)
			break;
	}

	if (prevcnt != data.cnt.cnt) {
		time_t time = data.cnt.cnt;
		data.time = *localtime(&time);
	}
}

IDLE_HANDLER() = &rtc_update;

uint32_t rtc_value()
{
	return data.cnt.cnt;
}

struct tm *rtc_time()
{
	return &data.time;
}
