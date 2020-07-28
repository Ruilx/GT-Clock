#include <device.h>
#include <debug.h>
#include <time.h>
#include <system/systick.h>
#include "rtc.h"

typedef union {
	uint32_t val;
	struct {
		uint16_t vall;
		uint16_t valh;
	};
} u32_t;

struct {
	u32_t cnt;
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
		u32_t presc;
		presc.val = 32768 - 1;		// 32k osc
		RTC->PRLL = presc.vall;
		RTC->PRLH = presc.valh;
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
	// Read RTC counter value
	u32_t cnt;
retry:	cnt.vall = RTC->CNTL;
	cnt.valh = RTC->CNTH;
	if (cnt.val == data.cnt.val)
		return;

	// Read RTC counter value again
	u32_t cnt2;
	cnt2.vall = RTC->CNTL;
	cnt2.valh = RTC->CNTH;
	// Make sure RTC counter did not change between readings
	if (cnt.val != cnt2.val)
		goto retry;

	// Update data and calculate calendar
	time_t time = data.cnt.val = cnt.val;
	data.time = *localtime(&time);
}

IDLE_HANDLER() = &rtc_update;

uint32_t rtc_value()
{
	return data.cnt.val;
}

struct tm *rtc_time()
{
	return &data.time;
}
