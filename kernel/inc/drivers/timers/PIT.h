#pragma once

#include "inc/common.h"

void PIT_init();

unsigned long long PIT_millis();

void PIT_irq();