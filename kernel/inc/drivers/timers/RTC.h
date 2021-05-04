#pragma once

#include "inc/common.h"

void RTC_init();

void RTC_get_time(int *h, int *m, int *s);

void RTC_irq();