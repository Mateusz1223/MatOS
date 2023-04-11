#include "inc/common.h"

#include "inc/UI/terminal.h"
#include "inc/drivers/VGA.h"

#include "inc/drivers/keyboard.h"
#include "inc/memory/heap.h"
#include "inc/drivers/system.h"
#include "inc/filesystem/filesystem.h"
#include "inc/filesystem/directory.h"

#define MAX_TERMINAL_NUM 5
#define MAX_INPUT_BUFFER_SIZE 500

Terminal debugTerminalBuffer;

static struct UIManagerStruct{
	unsigned char taskBar[485]; // displayed on the top of the screen
	int taskbarHeight;
	bool taskbarUpdated;

	int terminalNum;
	Terminal *terminals[MAX_TERMINAL_NUM];
	int currTerminal;

	char *termInputBuffers[MAX_TERMINAL_NUM];
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
	int y = term->cursorY - UIManager.terminals[UIManager.currTerminal]->displayY + UIManager.taskbarHeight;
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

static void change_current_terminal(int id){
	UIManager.currTerminal = id;
	UIManager.terminals[UIManager.currTerminal]->displayUpdated = true;

	for(int i=2; i<2*(3*MAX_TERMINAL_NUM + 5); i+=2)
		UIManager.taskBar[2*UIManager.width+i+1] = BACKGROUND_GREEN;

	UIManager.taskBar[2*UIManager.width+3+2*3*id] = GREEN;
	UIManager.taskBar[2*UIManager.width+5+2*3*id] = GREEN;

	UIManager.taskbarUpdated = true;
}

static void add_terminal(){
	UIManager.terminalNum++;
	int id = UIManager.terminalNum-1;
	UIManager.terminals[id] = (Terminal *)heap_malloc(sizeof(Terminal));
	terminal_init(UIManager.terminals[id], id, false);
	UIManager.termInputBuffers[id] = (char *)heap_malloc(MAX_INPUT_BUFFER_SIZE+5);
	terminal_print(UIManager.terminals[id], "> ");
	UIManager.termInputBuffers[id][0] = 0;
	terminal_scan(UIManager.terminals[id], UIManager.termInputBuffers[id], MAX_INPUT_BUFFER_SIZE);

	int firstPosition = 2*UIManager.width+id*6 + 2;
	UIManager.taskBar[firstPosition] = 'F';
	UIManager.taskBar[firstPosition+2] = (char)(id+1 + 48);
	UIManager.taskBar[firstPosition+4] = '|';
	if(UIManager.terminalNum == MAX_TERMINAL_NUM){
		UIManager.taskBar[firstPosition+6] = ' ';
	}
	else{
		UIManager.taskBar[firstPosition+6] = 'F';
		UIManager.taskBar[firstPosition+8] = '9';
		UIManager.taskBar[firstPosition+10] = '+';
		UIManager.taskBar[firstPosition+12] = '|';
	}

	change_current_terminal(id);

	//terminal_print(debugTerminal, "Added terminal, number of active terminals: %d\n", UIManager.terminalNum); // DEBUG
}

static void delete_terminal(int id){
	if(id >= UIManager.terminalNum || id < 1)
		return;
	UIManager.terminalNum--;
	heap_free(UIManager.terminals[id]);
	heap_free(UIManager.termInputBuffers[id]);
	for(int i=id; i<UIManager.terminalNum; i++){
		UIManager.terminals[i] = UIManager.terminals[i+1];
		UIManager.termInputBuffers[i] = UIManager.termInputBuffers[i+1];
	}
	if(id >= UIManager.terminalNum)
		id--;
	for(int i=2; i<2*(3*MAX_TERMINAL_NUM + 5); i+=2){
		UIManager.taskBar[2*UIManager.width+i] = ' ';
		UIManager.taskBar[2*UIManager.width+i+1] = BACKGROUND_GREEN;
	}
	for(int i=0; i<UIManager.terminalNum; i++){
		int firstPosition = 2*UIManager.width+i*6 + 2;
		UIManager.taskBar[firstPosition] = 'F';
		UIManager.taskBar[firstPosition+2] = (char)(i+1 + 48);
		UIManager.taskBar[firstPosition+4] = '|';
	}
	int firstPosition = 2*UIManager.width+(UIManager.terminalNum-1)*6+2;
	UIManager.taskBar[firstPosition+6] = 'F';
	UIManager.taskBar[firstPosition+8] = '9';
	UIManager.taskBar[firstPosition+10] = '+';
	UIManager.taskBar[firstPosition+12] = '|';
	change_current_terminal(id);

	//terminal_print(debugTerminal, "Deleted terminal, number of active terminals: %d\n", UIManager.terminalNum); // DEBUG
}

/*static void cd(){
	if(UIManager.termInputBuffers[UIManager.currTerminal][2] == '\0'){
		terminal_print(UIManager.terminals[UIManager.currTerminal], "Invalid syntax\n");
		return;
	}
	else{
		Directory *directory = filesystem_read_directory(&UIManager.termInputBuffers[UIManager.currTerminal][3], UIManager.terminals[UIManager.currTerminal]->volume,
														 UIManager.terminals[UIManager.currTerminal]->directoryCluster);
		int cluster = directory_find_directory(directory, name); //// Nooooooooooo !!!!!
		if(cluster == 0)
			terminal_print(UIManager.terminals[UIManager.currTerminal], "Invalid path\n");
		else{
			if(UIManager.terminals[UIManager.currTerminal]->volume[0] == '\0'){
				char *pointer = &UIManager.termInputBuffers[UIManager.currTerminal][3];
				while(pointer[0] == '\\' || pointer[0] == '/')
					pointer++;
				UIManager.terminals[UIManager.currTerminal]->volume[0] = pointer[0];
				UIManager.terminals[UIManager.currTerminal]->volume[1] = pointer[1];
			}
			UIManager.terminals[UIManager.currTerminal]->volumeCluster = cluster;
		}
	}
}*/

static void dir(){
	if(UIManager.termInputBuffers[UIManager.currTerminal][3] == '\0'){
		// Print relative directory
		Directory *directory = filesystem_read_directory("\\", UIManager.terminals[UIManager.currTerminal]->volume,
														 UIManager.terminals[UIManager.currTerminal]->directoryCluster);
		if(directory == 0)
			terminal_print(UIManager.terminals[UIManager.currTerminal], "Error while reading local directory\n");
		else{
			directory_print(directory, UIManager.terminals[UIManager.currTerminal]);
			directory_clear(directory);
			heap_free(directory);
		}
		terminal_print(UIManager.terminals[UIManager.currTerminal], "\n");
	}
	else{
		Directory *directory = filesystem_read_directory(&UIManager.termInputBuffers[UIManager.currTerminal][4],
														 UIManager.terminals[UIManager.currTerminal]->volume,
														 UIManager.terminals[UIManager.currTerminal]->directoryCluster);
		if(directory == 0)
			terminal_print(UIManager.terminals[UIManager.currTerminal], "Invalid path\n");
		else{
			directory_print(directory, UIManager.terminals[UIManager.currTerminal]);
			directory_clear(directory);
			heap_free(directory);
		}
		terminal_print(UIManager.terminals[UIManager.currTerminal], "\n");
	}
}

static void type(){
	if(UIManager.termInputBuffers[UIManager.currTerminal][4] == '\0')
		terminal_print(UIManager.terminals[UIManager.currTerminal], "Invalid syntax\n");
	else{
		char *file = filesystem_read_file(&UIManager.termInputBuffers[UIManager.currTerminal][5],
											UIManager.terminals[UIManager.currTerminal]->volume,
											UIManager.terminals[UIManager.currTerminal]->directoryCluster);
		if(file == 0)
			terminal_print(UIManager.terminals[UIManager.currTerminal], "Invalid path\n");
		else{
			terminal_print(UIManager.terminals[UIManager.currTerminal], file);
			heap_free(file);
		}
		terminal_print(UIManager.terminals[UIManager.currTerminal], "\n");
	}
}

//_____________________________________________________________________________

void UI_manager_init(){
	VGA_get_display_size(&UIManager.width, &UIManager.height); // Must be first

	UIManager.terminalNum = 1;

	UIManager.taskbarHeight = 3;

	UIManager.terminals[0] = &debugTerminalBuffer;
	debugTerminal = UIManager.terminals[0]; // debug terminal is in terminal.h
	terminal_init(debugTerminal, 0, true);
	UIManager.currTerminal = 0;

	// task bar
	for(int i=0; i<2*UIManager.width; i+=2){
		UIManager.taskBar[i] = '=';
		UIManager.taskBar[i+1] = BACKGROUND_GREEN;
	}
	for(int i=0; i<2*UIManager.width; i+=2){
		UIManager.taskBar[2*UIManager.width+i] = ' ';
		UIManager.taskBar[2*UIManager.width+i+1] = BACKGROUND_GREEN;
	}
	for(int i=0; i<2*UIManager.width; i+=2){
		UIManager.taskBar[4*UIManager.width+i] = '=';
		UIManager.taskBar[4*UIManager.width+i+1] = BACKGROUND_GREEN;
	}

	int firstPosition = 2*UIManager.width;
	UIManager.taskBar[firstPosition] = '|';
	UIManager.taskBar[firstPosition+(UIManager.width-1)*2] = '|';

	UIManager.taskBar[firstPosition+2] = 'F';
	UIManager.taskBar[firstPosition+3] = GREEN;
	UIManager.taskBar[firstPosition+4] = '1';
	UIManager.taskBar[firstPosition+5] = GREEN;
	UIManager.taskBar[firstPosition+6] = '|';
	UIManager.taskBar[firstPosition+8] = 'F';
	UIManager.taskBar[firstPosition+10] = '9';
	UIManager.taskBar[firstPosition+12] = '+';
	UIManager.taskBar[firstPosition+14] = '|';


	firstPosition = 3*UIManager.width; // UIManager.width / 2 * 2
	UIManager.taskBar[firstPosition-1] = BACKGROUND_GREEN | BROWN;
	UIManager.taskBar[firstPosition] = 'M';
	UIManager.taskBar[firstPosition+1] = BACKGROUND_GREEN | BROWN;
	UIManager.taskBar[firstPosition+2] = 'a';
	UIManager.taskBar[firstPosition+3] = BACKGROUND_GREEN | BROWN;
	UIManager.taskBar[firstPosition+4] = 't';
	UIManager.taskBar[firstPosition+5] = BACKGROUND_GREEN | BROWN;
	UIManager.taskBar[firstPosition+6] = 'O';
	UIManager.taskBar[firstPosition+7] = BACKGROUND_GREEN | BROWN;
	UIManager.taskBar[firstPosition+8] = 'S';
	UIManager.taskBar[firstPosition+9] = BACKGROUND_GREEN | BROWN;
	UIManager.taskBar[firstPosition+11] = BACKGROUND_GREEN | BROWN;

	// Time display
	firstPosition = 4*UIManager.width - 14;
	UIManager.taskBar[firstPosition] = '-';
	UIManager.taskBar[firstPosition+2] = '-';
	UIManager.taskBar[firstPosition+4] = ':';
	UIManager.taskBar[firstPosition+6] = '-';
	UIManager.taskBar[firstPosition+8] = '-';

	UIManager.taskbarUpdated = true;

	UIManager.cursorEnabled = true;

	terminal_print(debugTerminal, "[X] UI ready, screen size -> width: %d, height: %d\n", UIManager.width, UIManager.height);
}

void UI_manager_request_emergency_debug_terminal_display_update(){
	for(int i=2; i<2*(3*MAX_TERMINAL_NUM + 5); i+=2)
		UIManager.taskBar[2*UIManager.width+i+1] = BACKGROUND_GREEN;
	UIManager.taskBar[2*UIManager.width+3] = RED;
	UIManager.taskBar[2*UIManager.width+5] = RED;

	VGA_copy_to_textram(0, UIManager.taskBar, UIManager.taskbarHeight*UIManager.width); // Print taskbar
	update_terminal_display(debugTerminal);
}

void UI_manager_get_display_size(int *x, int *y){
	*x = UIManager.width;
	*y = UIManager.height - UIManager.taskbarHeight;
}

void UI_manager_PIT_irq_resident(){ // updates taskbar and terminal display

	if(UIManager.terminals[UIManager.currTerminal] != UIManager.terminals[0] &&!UIManager.terminals[UIManager.currTerminal]->scanInProgress && !UIManager.terminals[UIManager.currTerminal]->processInProgress){
		bool terminal_closed = false;
		if(UIManager.termInputBuffers[UIManager.currTerminal][0] == '\0');
		else if(strcmp(UIManager.termInputBuffers[UIManager.currTerminal], "help")){
			terminal_print(UIManager.terminals[UIManager.currTerminal],
				"Type:\n\tclose -> to close this terminal\n\tcls -> to clear screen\n\tdir [<path>] -> to list directory contents\n\tsys -> to show system informations\n\ttype [<path><filename>] -> to display the contents of a text file\n\n");
		}
		else if(strcmp(UIManager.termInputBuffers[UIManager.currTerminal], "cls")){
			terminal_clear(UIManager.terminals[UIManager.currTerminal]);
		}
		else if(strcmp(UIManager.termInputBuffers[UIManager.currTerminal], "sys")){
			system_print_informations(UIManager.terminals[UIManager.currTerminal]);
		}
		else if(strcmp(UIManager.termInputBuffers[UIManager.currTerminal], "close")){
			delete_terminal(UIManager.currTerminal);
			terminal_closed = true;
		}
		/*else if(strcmpn(UIManager.termInputBuffers[UIManager.currTerminal], "cd", 2)){
			cd();
		}*/
		else if(strcmpn(UIManager.termInputBuffers[UIManager.currTerminal], "dir", 3)){
			dir();
		}
		else if(strcmpn(UIManager.termInputBuffers[UIManager.currTerminal], "type", 4)){
			type();
		}
		else{
			terminal_set_color(UIManager.terminals[UIManager.currTerminal], LIGHT_RED);
			terminal_print(UIManager.terminals[UIManager.currTerminal], " \"%s\" is an unknown command\n\n", UIManager.termInputBuffers[UIManager.currTerminal]);
			terminal_set_color(UIManager.terminals[UIManager.currTerminal], LIGHT_GREEN);
		}
		if(!terminal_closed){
			terminal_print(UIManager.terminals[UIManager.currTerminal], "> ");
			UIManager.termInputBuffers[UIManager.currTerminal][0] = 0;
			terminal_scan(UIManager.terminals[UIManager.currTerminal], UIManager.termInputBuffers[UIManager.currTerminal], MAX_INPUT_BUFFER_SIZE);
		}
	}

	// If taskbar was updated copy new taskbar to VGA textram
	if(UIManager.taskbarUpdated)
		VGA_copy_to_textram(0, UIManager.taskBar, UIManager.taskbarHeight*UIManager.width); // Print taskbar
	
	if(UIManager.terminals[UIManager.currTerminal]->displayUpdated){
		update_terminal_display(UIManager.terminals[UIManager.currTerminal]);
		UIManager.terminals[UIManager.currTerminal]->displayUpdated = false;
	}
}

void UI_manager_keyboard_irq_resident( int keyId ) // will be called by keyboard_irq(), Scrolls terminal when Cursor Up or Don pressed, changes current terminal when F1/F2... pressed
{
	if(keyId == 73) // CURSOR UP
		terminal_scroll_up(UIManager.terminals[UIManager.currTerminal]);
	else if(keyId == 74) // CURSOR DOWN
		terminal_scroll_down(UIManager.terminals[UIManager.currTerminal]);
	else if(keyId == 72) // CURSOR LEFT
		terminal_move_cursor_left(UIManager.terminals[UIManager.currTerminal]);
	else if(keyId == 75) // CURSOR RIGHT
		terminal_move_cursor_right(UIManager.terminals[UIManager.currTerminal]);
	else if(2 <= keyId && keyId <= 6){ // F1 - > F5 because MAX_TERMINAL_NUM = 5, switch current terminal
		int index = keyId-2;
		if(index < UIManager.terminalNum)
			change_current_terminal(index);
	}
	else if(keyId == 10){ // F9, add new terminal
		if(UIManager.terminalNum < MAX_TERMINAL_NUM)
			add_terminal();
	}

	if(keyIdLookUpTable[keyId] == 8){
		terminal_backspace_in_scan_buffer(UIManager.terminals[UIManager.currTerminal]);
	}
	else if(keyIdLookUpTable[keyId] == 127){
		terminal_delete_in_scan_buffer(UIManager.terminals[UIManager.currTerminal]);
	}
	else if(keyIdLookUpTable[keyId] == -1){
		//terminal_print(debugTerminal, "\nUNDEFINED\n");
	}
	else if(keyIdLookUpTable[keyId] == -3){	
		//terminal_print(debugTerminal, "\n");
		terminal_end_scan(UIManager.terminals[UIManager.currTerminal]);
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
		terminal_add_char_to_scan_buffer(UIManager.terminals[UIManager.currTerminal], ch);
	}
}

void UI_manager_RTC_irq_resident(int h, int m) // will be called by RTC_irq(), updates taskbar time
{


	int firstPosition = 4*UIManager.width - 14;

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