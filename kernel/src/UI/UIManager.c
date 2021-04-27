#include "inc/common.h"

#include "inc/UI/terminal.h"

#include "inc/drivers/VGA.h"
#include "inc/drivers/keyboard.h"

#define TERMINAL_NUM 2

static struct UIManagerStruct
{
	unsigned char taskBar[320]; // displayed on the top of the screen
	int taskbarHeight;
	bool taskbarUpdated;

	Terminal terminals[TERMINAL_NUM];
	Terminal *currentTerminal;
	int width;
	int height;
}UIManager;

//_____________________________________________________________________________

static void update_display()
{
	VGA_copy_to_textram(UIManager.taskbarHeight*UIManager.width,
						&UIManager.currentTerminal->buffer[UIManager.currentTerminal->displayY * UIManager.currentTerminal->width * 2],
						UIManager.currentTerminal->height * UIManager.width);

	int x = UIManager.currentTerminal->cursorX;
	int y = UIManager.currentTerminal->cursorY - UIManager.currentTerminal->displayY + UIManager.taskbarHeight;
	if(y >= UIManager.height || y < UIManager.taskbarHeight)
	{
		VGA_disable_cursor();
	}
	else
	{
		VGA_enable_cursor();
		VGA_set_cursor_position(x, y);
	}
}

//_____________________________________________________________________________

void UI_manager_init()
{
	VGA_get_display_size(&UIManager.width, &UIManager.height); // Must be first

	UIManager.taskbarHeight = 2;

	debugTerminal = &UIManager.terminals[0]; // debug terminal is in terminal.h

	for(int i=0; i<TERMINAL_NUM; i++)
		terminal_init(&UIManager.terminals[i]);

	UIManager.currentTerminal = &UIManager.terminals[0];

	terminal_set_color(UIManager.currentTerminal, RED | BLINKING);
	terminal_print(UIManager.currentTerminal, "This terminal is currently a debug terminal\n\n");
	terminal_set_color(UIManager.currentTerminal, LIGH_GREEN);

	// task bar
	int n = 2*UIManager.taskbarHeight*UIManager.width;
	for(int i=0; i<n; i+=2)
	{
		if(i >= 2*(UIManager.taskbarHeight-1)*UIManager.width)
			UIManager.taskBar[i] = '#';
		else
			UIManager.taskBar[i] = ' ';

		UIManager.taskBar[i+1] = BACKGROUND_GREEN;

	}

	UIManager.taskBar[0] = '|';
	UIManager.taskBar[(UIManager.width-1)*2] = '|';

	UIManager.taskBar[2] = 'F';
	UIManager.taskBar[3] = GREEN;
	UIManager.taskBar[4] = '1';
	UIManager.taskBar[5] = GREEN;
	UIManager.taskBar[6] = '|';
	UIManager.taskBar[8] = 'F';
	UIManager.taskBar[9] = BACKGROUND_GREEN;
	UIManager.taskBar[10] = '2';
	UIManager.taskBar[11] = BACKGROUND_GREEN;
	UIManager.taskBar[12] = '|';

	int firstPosition = UIManager.width; // UIManager.width / 2 * 2
	UIManager.taskBar[firstPosition-1] = BACKGROUND_CYAN;
	UIManager.taskBar[firstPosition] = 'M';
	UIManager.taskBar[firstPosition+1] = BACKGROUND_CYAN;
	UIManager.taskBar[firstPosition+2] = 'a';
	UIManager.taskBar[firstPosition+3] = BACKGROUND_CYAN;
	UIManager.taskBar[firstPosition+4] = 't';
	UIManager.taskBar[firstPosition+5] = BACKGROUND_CYAN;
	UIManager.taskBar[firstPosition+6] = 'O';
	UIManager.taskBar[firstPosition+7] = BACKGROUND_CYAN;
	UIManager.taskBar[firstPosition+8] = 'S';
	UIManager.taskBar[firstPosition+9] = BACKGROUND_CYAN;
	UIManager.taskBar[firstPosition+11] = BACKGROUND_CYAN;

	UIManager.taskbarUpdated = true;
}

void UI_manager_run()
{
	for(;;)
	{
		// Update time on task bar
		//...

		// If taskbar was updated copy new taskbar to VGA textram
		if(UIManager.taskbarUpdated)
			VGA_copy_to_textram(0, UIManager.taskBar, UIManager.taskbarHeight*UIManager.width); // Print taskbar
		
		if(UIManager.currentTerminal->displayUpdated)
		{
			update_display();
			UIManager.currentTerminal->displayUpdated = false;
		}
	}
}

void UI_manager_keyboard_irq_resident( int keyId ) // will be called by keyboard_irq(), Scrolls terminal when Cursor Up or Don pressed, changes current terminal when F1/F2... pressed
{
	if(keyId == 73) // CURSOR UP
	{
		terminal_scroll_up(UIManager.currentTerminal);
	}
	else if(keyId == 74) // CURSOR DOWN
	{
		terminal_scroll_down(UIManager.currentTerminal);
	}
	else if(keyId == 2) // F1
	{
		UIManager.currentTerminal = &UIManager.terminals[0];
		UIManager.currentTerminal->displayUpdated = true;

		UIManager.taskBar[2] = 'F';
		UIManager.taskBar[3] = GREEN;
		UIManager.taskBar[4] = '1';
		UIManager.taskBar[5] = GREEN;
		UIManager.taskBar[6] = '|';
		UIManager.taskBar[8] = 'F';
		UIManager.taskBar[9] = BACKGROUND_GREEN;
		UIManager.taskBar[10] = '2';
		UIManager.taskBar[11] = BACKGROUND_GREEN;
		UIManager.taskBar[12] = '|';

		UIManager.taskbarUpdated = true;
	}
	else if(keyId == 3) // F2
	{
		UIManager.currentTerminal = &UIManager.terminals[1];
		UIManager.currentTerminal->displayUpdated = true;

		UIManager.taskBar[2] = 'F';
		UIManager.taskBar[3] = BACKGROUND_GREEN;
		UIManager.taskBar[4] = '1';
		UIManager.taskBar[5] = BACKGROUND_GREEN;
		UIManager.taskBar[6] = '|';
		UIManager.taskBar[8] = 'F';
		UIManager.taskBar[9] = GREEN;
		UIManager.taskBar[10] = '2';
		UIManager.taskBar[11] = GREEN;
		UIManager.taskBar[12] = '|';

		UIManager.taskbarUpdated = true;
	}

	// Debug

	if(keyIdLookUpTable[keyId] == 8);
		//terminal_print(debugTerminal, "\nBACKSPACE\n");
	else if(keyIdLookUpTable[keyId] == 127);
		//terminal_print(debugTerminal, "\nDELETE\n");
	else if(keyIdLookUpTable[keyId] == -1);
		//terminal_print(debugTerminal, "\nUNDEFINED\n");
	else if(keyIdLookUpTable[keyId] == -3)
	{
		//terminal_print(debugTerminal, "\nENTER\n");
		
		terminal_print(debugTerminal, "\n");
	}
	else if(keyIdLookUpTable[keyId] == -4)
	{
		;
		//terminal_print(debugTerminal, "\n<-\n");
	}
	else if(keyIdLookUpTable[keyId] == -5);
		//terminal_print(debugTerminal, "\n->\n");
	else
		terminal_putchar(debugTerminal, keyIdLookUpTable[keyId]);
}

void UI_manager_get_display_size(int *x, int *y)
{
	*x = UIManager.width;
	*y = UIManager.height - UIManager.taskbarHeight;
}