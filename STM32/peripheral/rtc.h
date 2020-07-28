#pragma once

#include <stdint.h>
#include <time.h>

void rtc_init();
uint32_t rtc_value();
struct tm *rtc_time();
