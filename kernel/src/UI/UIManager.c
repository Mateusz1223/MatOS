#include "inc/common.h"

#include "inc/UI/terminal.h"

#include "inc/drivers/VGA.h"
#include "inc/drivers/keyboard.h"

#define TERMINAL_NUM 2

static struct UIManagerStruct{
	unsigned char taskBar[320]; // displayed on the top of the screen
	int taskbarHeight;
	bool taskbarUpdated;

	int terminalCount;
	Terminal terminals[TERMINAL_NUM];
	Terminal *currentTerminal;

	bool termInputBufferFilled[TERMINAL_NUM];
	char termInputBuffers[TERMINAL_NUM][500]; // Should be alocated dynamically in the future
	int width;
	int height;

	bool cursorEnabled;
}UIManager;

//_____________________________________________________________________________

static void update_terminal_display(Terminal *term){
	VGA_copy_to_textram(UIManager.taskbarHeight*UIManager.width,
						&term->buffer[term->displayY * term->width * 2],
						term->height * UIManager.width);

	int x = term->cursorX;
	int y = term->cursorY - UIManager.currentTerminal->displayY + UIManager.taskbarHeight;
	if(term->cursorEnabled == false || y >= UIManager.height || y < UIManager.taskbarHeight){
		if(UIManager.cursorEnabled == true){
			UIManager.cursorEnabled = false;
			VGA_disable_cursor();
		}
	}
	else{
		if(UIManager.cursorEnabled == false){
			UIManager.cursorEnabled = true;
			VGA_enable_cursor();
		}
	}

	VGA_set_cursor(x, y, term->color);
}

//_____________________________________________________________________________

void UI_manager_init(){
	VGA_get_display_size(&UIManager.width, &UIManager.height); // Must be first

	UIManager.terminalCount = TERMINAL_NUM;

	UIManager.taskbarHeight = 2;

	debugTerminal = &UIManager.terminals[0]; // debug terminal is in terminal.h
	terminal_init(debugTerminal, true);

	for(int i=1; i<UIManager.terminalCount; i++){
		terminal_init(&UIManager.terminals[i], false);
		UIManager.termInputBuffers[i][0] = 0;
		UIManager.termInputBufferFilled[i] = false;
	}

	UIManager.currentTerminal = &UIManager.terminals[0];

	// task bar
	int n = 2*UIManager.taskbarHeight*UIManager.width;
	for(int i=0; i<n; i+=2){
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

	firstPosition = UIManager.width * 2 - 14;
	UIManager.taskBar[firstPosition] = '-';
	UIManager.taskBar[firstPosition+2] = '-';
	UIManager.taskBar[firstPosition+4] = ':';
	UIManager.taskBar[firstPosition+6] = '-';
	UIManager.taskBar[firstPosition+8] = '-';

	UIManager.taskbarUpdated = true;

	UIManager.cursorEnabled = true;

	//terminal_print(debugTerminal, "Screen size -> width: %d, height: %d\n", UIManager.width, UIManager.height);
}

void UI_manager_request_emergency_debug_terminal_display_update(){
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

	VGA_copy_to_textram(0, UIManager.taskBar, UIManager.taskbarHeight*UIManager.width); // Print taskbar
	update_terminal_display(debugTerminal);
}

void UI_manager_get_display_size(int *x, int *y){
	*x = UIManager.width;
	*y = UIManager.height - UIManager.taskbarHeight;
}

void UI_manager_PIT_irq_resident() // updates taskbar and terminal display
{
	for(int i=1; i<UIManager.terminalCount; i++){
		if(&UIManager.terminals[i] == debugTerminal) // debug terminal don't interact with user, just prints debug info
			continue;
			
		if(UIManager.terminals[i].processInProgress)
			continue;

		if(UIManager.terminals[i].scanInProgress == false){
			if(strcmp(UIManager.termInputBuffers[i], "help")){
				terminal_print(&UIManager.terminals[i], " Type:\n\thelp -> to see help message\n\tcls -> to clear screen\n\tno more features yet\n\n");
			}
			else if(strcmp(UIManager.termInputBuffers[i], "cls")){
				terminal_clear(&UIManager.terminals[i]);
			}
			else if(UIManager.termInputBufferFilled[i] == true){
				terminal_set_color(&UIManager.terminals[i], LIGHT_RED);
				terminal_print(&UIManager.terminals[i], " \"%s\" is an unknown command\n\n", UIManager.termInputBuffers[i]);
				terminal_set_color(&UIManager.terminals[i], LIGHT_GREEN);
			}
			UIManager.termInputBufferFilled[i] = false;

			terminal_print(&UIManager.terminals[i], "> ");
			terminal_scan(&UIManager.terminals[i], UIManager.termInputBuffers[i], 500);
			UIManager.termInputBufferFilled[i] = true;
		}
	}

	// If taskbar was updated copy new taskbar to VGA textram
	if(UIManager.taskbarUpdated)
		VGA_copy_to_textram(0, UIManager.taskBar, UIManager.taskbarHeight*UIManager.width); // Print taskbar
	
	if(UIManager.currentTerminal->displayUpdated){
		update_terminal_display(UIManager.currentTerminal);
		UIManager.currentTerminal->displayUpdated = false;
	}
}

void UI_manager_keyboard_irq_resident( int keyId ) // will be called by keyboard_irq(), Scrolls terminal when Cursor Up or Don pressed, changes current terminal when F1/F2... pressed
{
	if(keyId == 73) // CURSOR UP
		terminal_scroll_up(UIManager.currentTerminal);
	else if(keyId == 74) // CURSOR DOWN
		terminal_scroll_down(UIManager.currentTerminal);
	else if(keyId == 72) // CURSOR LEFT
		terminal_move_cursor_left(UIManager.currentTerminal);
	else if(keyId == 75) // CURSOR RIGHT
		terminal_move_cursor_right(UIManager.currentTerminal);
	else if(keyId == 2){ // F1
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
	else if(keyId == 3){ // F2
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

	if(keyIdLookUpTable[keyId] == 8){
		terminal_backspace_in_scan_buffer(UIManager.currentTerminal);
	}
	else if(keyIdLookUpTable[keyId] == 127){
		terminal_delete_in_scan_buffer(UIManager.currentTerminal);
	}
	else if(keyIdLookUpTable[keyId] == -1){
		//terminal_print(debugTerminal, "\nUNDEFINED\n");
	}
	else if(keyIdLookUpTable[keyId] == -3){	
		//terminal_print(debugTerminal, "\n");
		terminal_end_scan(UIManager.currentTerminal);
	}
	else if(keyIdLookUpTable[keyId] == -4){
		;
		//terminal_print(debugTerminal, "\n<-\n");
	}
	else if(keyIdLookUpTable[keyId] == -5){
		//terminal_print(debugTerminal, "\n->\n");
	}
	else{
		bool upperCase = false;
		bool shiftPressed = keyboard_is_key_pressed(56) || keyboard_is_key_pressed(67);
		if(shiftPressed)
			upperCase = true;

		if(keyboard_is_caps_lock()){
			if(upperCase == true)
				upperCase = false;
			else
				upperCase = true;
		}
		
		char ch = keyIdLookUpTable[keyId];

		if(97 <= (int)ch && (int)ch <= 122 && upperCase)
			ch -= 32;
		else if(48 <= (int)ch && (int)ch <= 57 && shiftPressed){
			char translation[] = {')','!','@','#','$','%','^','&','*','('};
			ch = translation[(int)ch - 48];
		}
		else if(shiftPressed){ // lookup table may be a better solution
			switch(ch){
			case '`':
				ch = '~';
			break;

			case '-':
				ch = '_';
			break;

			case '=':
				ch = '+';
			break;

			case '[':
				ch = '{';
			break;

			case ']':
				ch = '}';
			break;

			case ';':
				ch = ':';
			break;

			case '\'':
				ch = '"';
			break;

			case '\\':
				ch = '|';
			break;

			case ',':
				ch = '<';
			break;

			case '.':
				ch = '>';
			break;

			case '/':
				ch = '?';
			break;
			}
		}

		//terminal_putchar(debugTerminal, keyIdLookUpTable[keyId]);
		terminal_add_char_to_scan_buffer(UIManager.currentTerminal, ch);
	}
}

void UI_manager_RTC_irq_resident(int h, int m) // will be called by RTC_irq(), updates taskbar time
{


	int firstPosition = UIManager.width * 2 - 14;

	if(h < 10){
		UIManager.taskBar[firstPosition] = '0';
		UIManager.taskBar[firstPosition+2] = (char)(h + 48);
	}
	else{
		UIManager.taskBar[firstPosition] = (char)((h / 10) + 48);
		UIManager.taskBar[firstPosition+2] = (char)((h % 10) + 48); // May be optimized using previous calculation h / 10
	}

	if(m < 10){
		UIManager.taskBar[firstPosition+6] = '0';
		UIManager.taskBar[firstPosition+8] = (char)(m + 48);
	}
	else{
		UIManager.taskBar[firstPosition+6] = (char)((m / 10) + 48);
		UIManager.taskBar[firstPosition+8] = (char)((m % 10) + 48); // May be optimized using previous calculation h / 10
	}
}