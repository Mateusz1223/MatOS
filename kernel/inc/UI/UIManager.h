#pragma once

#include "inc/common.h"

#include "inc/UI/terminal.h"

void UI_manager_init();

void UI_manager_run();

void UI_manager_keyboard_irq_resident( int keyId ); // will be called by keyboard_irq(), Scrolls terminal when Cursor Up or Don pressed, changes current terminal when F1/F2... pressed

void UI_manager_get_display_size(int *x, int *y);