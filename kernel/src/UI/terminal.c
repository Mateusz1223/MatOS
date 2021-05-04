#include "inc/UI/terminal.h"

#include "inc/UI/UIManager.h"
#include "inc/drivers/VGA.h" // just for colors

//___________________________________________________________________________________________________

static void check_last_character_y( Terminal *this)
{
	if(this->lastCharacterY >= this->displayY+this->height)
		terminal_scroll_down(this);

	if(this->lastCharacterY >= this->buffSize / this->width - 1)
	{
		memmove(this->buffer, &this->buffer[this->buffSize], (size_t)this->buffSize); // this->buffSize / 2 * 2, will always be even number, so won't cause problems

		for(int i=this->buffSize; i<this->buffSize*2; i+=2)
		{
			this->buffer[i] = ' ';
			this->buffer[i+1] = this->color;
		}	

		this->cursorY -= this->buffSize / this->width / 2;

		this->displayY -= this->buffSize / this->width / 2;
		this->displayY++;
		this->lastCharacterY -= this->buffSize / this->width / 2;

		this->displayUpdated = true;
	}
}

static void next_line( Terminal *this)
{
	this->cursorY++;
	this->cursorX = 0;

	this->lastCharacterY = this->cursorY;
	check_last_character_y(this);
}

static void put_character( Terminal *this, char ch)
{
	switch(ch)
		{
			case '\n':
			{
				next_line(this);
				break;
			}
			
			case '\r':
			{
				next_line(this);
				break;
			}
		
			case '\t':
			{
				for(int i=0; i<4; i++)
					put_character(this, ' ');
				break;
			}
			
			default:
			{
				int pos = (this->cursorY * this->width + this->cursorX)*2;
				
				this->buffer[pos] = ch;
				this->buffer[pos+1] = this->color;

				this->cursorX++;
				if(this->cursorX >= this->width)
					next_line(this);
			}
		}
}

static void put_text( Terminal *this, char *str )
{
	while(*str != '\0')
	{
		put_character(this, *str);
		++str;
	}
}

static void print_uint( Terminal *this, unsigned int d )
{
	if(d==0)
	{
		put_character(this, '0');
		return;
	}
	
	char buf[12]={'\0'};
	char *p = &buf[11];
	
	while(d!=0)
	{
		--p;
		*p = (d % 10) + '0';
		d /= 10;
	}
	put_text(this, p);
}

static void print_int( Terminal *this, int d )
{	
	if(d<0)
	{
		d=-d;
		put_character(this, '-');
	}
	
	print_uint(this, (unsigned int) d);
}

static void print_hex( Terminal *this, unsigned int d ) //Seem to be fine but used to crush system
{
	put_text(this, "0x");
	
	if(d==0)
	{
		put_character(this, '0');
		return;
	}
	
	int i = 0;
	
	while((d>>28)==0)
	{
		d<<=4;
		++i;
	}
	
	while(i!=8)
	{
		put_character(this,  "0123456789abcdef"[d>>28]);
		d<<=4;
		++i;
	}
}

//___________________________________________________________________________________________________

void terminal_init( Terminal *this )
{
	this->buffSize = 4000; // in the future buffer will be dynamically allocated (it will be buffer size in bytes divided by 2)

	UI_manager_get_display_size(&this->width, &this->height);
	this->cursorX = 0;
	this->cursorY = 0;
	this->cursorEnabled = true;
	this->color = LIGH_GREEN;

	// clear buffer
	for(int i=0; i<this->buffSize*2; i+=2)
	{
		this->buffer[i] = ' ';
		this->buffer[i+1] = this->color;
	}

	this->displayY = 0;
	this->lastCharacterY = 0;

	terminal_print(this, "MATOS https://github.com/Mateusz1223/MatOS\nMateusz Piasecki https://piaseckimateusz.pl/\n\nWelcome to Terminal !!\n\n");
	this->displayUpdated = true;
}

void terminal_print( Terminal *this, const char *str, ... ) // May not work properly. May mix order
{
	va_list args;
	va_start(args, str); // ????
	
	while(*str != '\0')
	{
		if(*str=='%')
		{
			++str;
			switch(*str)
			{
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
}

void terminal_putchar( Terminal *this, char ch )
{
	char str[2];
	str[0] = ch;
	str[1] = 0;
	terminal_print(this, str);
}

void terminal_set_color( Terminal *this, char ch )
{
	this->color = ch;

	int pos = (this->cursorY * this->width + this->cursorX)*2 + 1;
	this->buffer[pos] = ch;
}

void terminal_scan( Terminal *this, const char *str, ... )
{
	// TODO
}

void terminal_scroll_up( Terminal *this )
{
	this->displayY--;
	if(this->displayY < 0)
		this->displayY = 0;

	this->displayUpdated = true;
}

void terminal_scroll_down(Terminal *this)
{
	this->displayY++;
	if(this->displayY + this->height >= this->buffSize / this->width)
		this->displayY--;

	this->displayUpdated = true;
}