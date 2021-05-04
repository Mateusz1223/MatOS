#pragma once

#include "inc/common.h"

#include "inc/UI/terminal.h"

void UI_manager_init();

void UI_manager_get_display_size(int *x, int *y);

// Used for unexpected interrupts such as division by 0, to print "message of death"
void UI_manager_request_emergency_display_update(Terminal *term);

// Residents

void UI_manager_PIT_irq_resident(); // updates taskbar and terminal display

void UI_manager_keyboard_irq_resident( int keyId ); // will be called by keyboard_irq(), Scrolls terminal when Cursor Up or Don pressed, changes current terminal when F1/F2... pressed

void UI_manager_RTC_irq_resident(int h, int m); // will be called by RTC_irq(), updates taskbar time