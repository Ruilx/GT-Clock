#pragma once

#include <stdint.h>
#include <time.h>
#include <list.h>

// RTC second timer callback
typedef void (*const rtc_second_handler_t)(uint32_t time);
#define RTC_SECOND_HANDLER()	LIST_ITEM(rtc_second, rtc_second_handler_t)

void rtc_init();

time_t rtc_timestamp();
void rtc_set_timestamp(time_t time);

struct tm *rtc_time();
void rtc_set_time(struct tm *dtm);
