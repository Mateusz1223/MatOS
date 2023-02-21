#include "inc/UI/terminal.h"

#include "inc/UI/UIManager.h"
#include "inc/drivers/VGA.h" // just for colors
#include "inc/memory/heap.h"

//___________________________________________________________________________________________________

static void check_last_character_y(Terminal *this){
	if(this->lastCharacterY >= this->displayY+this->height)
		terminal_scroll_down(this);

	if(this->lastCharacterY >= this->bufferSize / this->width - 1){
		memmove(this->buffer, &this->buffer[2*this->pageSize], (size_t)(2*this->bufferSize - 2*this->pageSize)); // this->bufferSize / 2 * 2, will always be even number, so won't cause problems

		for(int i=(2*this->bufferSize-2*this->pageSize); i<this->bufferSize*2; i+=2){
			this->buffer[i] = ' ';
			this->buffer[i+1] = this->color;
		}

		this->cursorY -= this->pageSize / this->width;

		this->displayY = this->cursorY - 15;
		this->lastCharacterY -= this->pageSize / this->width;

		this->scanBuffer.pos -=  this->pageSize;

		this->displayUpdated = true;
	}
}

static void next_line(Terminal *this){
	this->cursorY++;
	this->cursorX = 0;

	this->lastCharacterY = this->cursorY;
	check_last_character_y(this);
}

static void put_character(Terminal *this, char ch){
	switch(ch){
		case '\n':{
			next_line(this);
			break;
		}
		
		case '\r':{
			next_line(this);
			break;
		}
	
		case '\t':{
			for(int i=0; i<4; i++)
				put_character(this, ' ');
			break;
		}
		
		default:{
			int pos = (this->cursorY * this->width + this->cursorX)*2;
			
			this->buffer[pos] = ch;
			this->buffer[pos+1] = this->color;

			this->cursorX++;
			if(this->cursorX >= this->width)
				next_line(this);
		}
	}
}

static void put_text(Terminal *this, char *str){
	while(*str != '\0'){
		put_character(this, *str);
		++str;
	}
}

static void print_uint(Terminal *this, unsigned int d){
	if(d==0){
		put_character(this, '0');
		return;
	}
	
	char buf[12]={'\0'};
	char *p = &buf[11];
	
	while(d!=0){
		--p;
		*p = (d % 10) + '0';
		d /= 10;
	}
	put_text(this, p);
}

static void print_int(Terminal *this, int d){	
	if(d<0){
		d=-d;
		put_character(this, '-');
	}
	
	print_uint(this, (unsigned int) d);
}

static void print_hex(Terminal *this, unsigned int d){ //Seem to be fine but used to crush system
	put_text(this, "0x");
	
	if(d==0){
		put_character(this, '0');
		return;
	}
	
	int i = 0;
	
	while((d>>28)==0){
		d<<=4;
		++i;
	}
	
	while(i!=8){
		put_character(this,  "0123456789abcdef"[d>>28]);
		d<<=4;
		++i;
	}
}

static void print_bool(Terminal *this, bool b){
	if(b)
		put_text(this, "true");
	else
		put_text(this, "false");
}

void terminal_scan(Terminal *this, char *buff, size_t size){
	this->scanInProgress = true;

	this->scanBuffer.buffer = buff;
	this->scanBuffer.size = size;
	if(size > 500)
		this->scanBuffer.size = 500;

	this->scanBuffer.pos = this->cursorY*this->width + this->cursorX;

	this->scanBuffer.lastCharacter = 0;
}

//___________________________________________________________________________________________________

void terminal_init(Terminal *this, int id, bool debugMode){
	this->id = id;
	
	this->bufferSize = 10000; // * 2 bytes (8 bits character + 8 bits color)
	UI_manager_get_display_size(&this->width, &this->height);
	this->pageSize = this->width*this->height;

	this->cursorEnabled = true;
	this->color = LIGHT_GREEN;

	terminal_clear(this);

	this->processInProgress = false;
	this->scanInProgress = false;

	terminal_print(this, "MATOS https://github.com/Mateusz1223/MatOS\nMateusz Piasecki https://piaseckimateusz.pl/\n\nWelcome to Terminal !!\n\n");

	if(debugMode){
		terminal_set_color(this, LIGHT_RED | BLINKING);
		terminal_print(this, "This terminal is currently a debug terminal\n\n");
		terminal_set_color(this, LIGHT_GREEN);
		this->cursorEnabled = false;
	}
	else{
		terminal_print(this, "For help type 'help'\n");
	}

	this->displayUpdated = true;
}

int terminal_print(Terminal *this, const char *str, ...) // May not work properly. May mix order
{
	if(this->scanInProgress)
		return 0;

	va_list args;
	va_start(args, str); // ????
	
	while(*str != '\0'){
		if(*str=='%'){
			++str;
			switch(*str){
				case 'i':
				case 'd':
					print_int(this, va_arg(args, int));
					break;
				case 'x':
					print_hex(this, va_arg(args, int));
					break;
				case 'u':
					print_uint(this, va_arg(args, unsigned int));
					break;
				case 's':
					put_text(this, va_arg(args, char*));
					break;
				case 'b':
					print_bool(this, (bool)va_arg(args, int)); // variables are converted to int when too small in va
					break;
				case '%':
					put_character(this, '%');
					break;
				default:
					put_character(this, *str);
					break;
			}
		}else put_character(this, *str);
		
		++str;
	}
	
	va_end(args);

	this->displayUpdated = true;

	return 1;
}

void terminal_clear(Terminal *this){
	this->cursorX = 0;
	this->cursorY = 0;

	// clear buffer
	for(int i=0; i<this->bufferSize*2; i+=2){
		this->buffer[i] = ' ';
		this->buffer[i+1] = this->color;
	}

	this->displayY = 0;
	this->lastCharacterY = 0;
}

void terminal_putchar(Terminal *this, char ch){
	char str[2];
	str[0] = ch;
	str[1] = 0;
	terminal_print(this, str);
}

void terminal_set_color(Terminal *this, char ch){
	this->color = ch;

	int pos = (this->cursorY * this->width + this->cursorX)*2 + 1;
	this->buffer[pos] = ch;
}

// Functions for UI manager keyboard irq resident

void terminal_scroll_up(Terminal *this){
	this->displayY--;
	if(this->displayY < 0)
		this->displayY = 0;

	this->displayUpdated = true;
}

void terminal_scroll_down(Terminal *this){
	this->displayY++;
	if(this->displayY + this->height >= this->bufferSize / this->width)
		this->displayY--;

	this->displayUpdated = true;
}

void terminal_move_cursor_left(Terminal *this){
	if(!this->scanInProgress)
		return;

	if(this->scanBuffer.pos < this->cursorY*this->width + this->cursorX)
	{
		this->cursorX--;
		if(this->cursorX < 0)
		{
			this->cursorX += this->width;
			this->cursorY --;
		}
	}
	this->displayUpdated = true;
}

void terminal_move_cursor_right(Terminal *this){
	if(!this->scanInProgress)
		return;

	if(this->cursorY*this->width + this->cursorX < this->scanBuffer.pos + this->scanBuffer.lastCharacter)
	{
		this->cursorX++;
		if(this->cursorX >= this->width)
		{
			this->cursorX -= this->width;
			this->cursorY++;
		}
	}
	this->displayUpdated = true;
}

void terminal_add_char_to_scan_buffer(Terminal *this, char ch){
	if(!this->scanInProgress)
		return;

	if(ch == '\t'){
		for(int i=0; i<4; i++)
			terminal_add_char_to_scan_buffer(this, ' ');
		return;
	}

	if(this->scanBuffer.lastCharacter >= this->scanBuffer.size-1)
		return;

	int cursorPos = this->cursorY*this->width + this->cursorX;

	// move chars in scanBuffer buffer
	memmove(&this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos + 1], &this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos], (size_t)(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos));
	// move chars in Terminal buffer
	memmove(&this->buffer[2*(cursorPos+1)], &this->buffer[2*cursorPos], (size_t)(2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos)));

	this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos] = ch;
	this->buffer[2*cursorPos] = ch;
	this->buffer[2*cursorPos + 1] = this->color;

	this->cursorX++;
	if(this->cursorX >= this->width){
		this->cursorX -= this->width;
		this->cursorY++;
	}
	this->scanBuffer.lastCharacter++;
	this->lastCharacterY = (this->scanBuffer.pos + this->scanBuffer.lastCharacter) / this->width;
	check_last_character_y(this);

	this->displayUpdated = true;
}

void terminal_backspace_in_scan_buffer(Terminal *this){
	if(!this->scanInProgress)
		return;

	int cursorPos = this->cursorY*this->width + this->cursorX;

	if(cursorPos - this->scanBuffer.pos == 0)
		return;

	// move chars in scanBuffer buffer
	memmove(&this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos - 1], &this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos], (size_t)(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos));
	// move chars in Terminal buffer
	memmove(&this->buffer[2*(cursorPos-1)], &this->buffer[2*cursorPos], (size_t)(2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos))); //memmove(&this->buffer[2*cursorPos], &this->buffer[2*(cursorPos+1)], (size_t)(2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos)));

	this->scanBuffer.buffer[this->scanBuffer.lastCharacter-1] = ' ';
	this->buffer[2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter-1)] = ' ';

	this->cursorX--;
	if(this->cursorX < 0)
	{
		this->cursorX += this->width;
		this->cursorY --;
	}

	this->scanBuffer.lastCharacter--;
	this->lastCharacterY = (this->scanBuffer.pos + this->scanBuffer.lastCharacter) / this->width;
	check_last_character_y(this);

	this->displayUpdated = true;
}

void terminal_delete_in_scan_buffer(Terminal *this){
	if(!this->scanInProgress)
		return;

	int cursorPos = this->cursorY*this->width + this->cursorX;

	if(cursorPos == this->scanBuffer.pos + this->scanBuffer.lastCharacter)
		return;

	// move chars in scanBuffer buffer
	memmove(&this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos], &this->scanBuffer.buffer[cursorPos - this->scanBuffer.pos + 1], (size_t)(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos - 1));
	// move chars in Terminal buffer
	memmove(&this->buffer[2*cursorPos], &this->buffer[2*(cursorPos + 1)], (size_t)(2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos - 1))); //memmove(&this->buffer[2*cursorPos], &this->buffer[2*(cursorPos+1)], (size_t)(2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter - cursorPos)));

	this->scanBuffer.buffer[this->scanBuffer.lastCharacter-1] = ' ';
	this->buffer[2*(this->scanBuffer.pos + this->scanBuffer.lastCharacter-1)] = ' ';

	this->scanBuffer.lastCharacter--;
	this->lastCharacterY = (this->scanBuffer.pos + this->scanBuffer.lastCharacter) / this->width;
	check_last_character_y(this);

	this->displayUpdated = true;
}

void terminal_end_scan(Terminal *this){
	if(!this->scanInProgress)
		return;

	this->scanBuffer.buffer[this->scanBuffer.lastCharacter] = (char)0;

	this->cursorY = (this->scanBuffer.pos + this->scanBuffer.lastCharacter) / this->width;
	this->cursorX = (this->scanBuffer.pos + this->scanBuffer.lastCharacter) % this->width;

	this->scanInProgress = false;

	terminal_putchar(this, '\n');
}

// Threads

void terminal_debug_thread(Terminal *this);
void terminal_classic_thread(Terminal *this);