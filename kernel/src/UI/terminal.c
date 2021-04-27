#include "inc/UI/terminal.h"

#include "inc/UI/UIManager.h"
#include "inc/drivers/VGA.h" // just for colors

//___________________________________________________________________________________________________

static void check_last_character_y( Terminal *term)
{
	if(term->lastCharacterY >= term->displayY+term->height)
		terminal_scroll_down(term);

	if(term->lastCharacterY >= term->buffSize / term->width - 1)
	{
		memmove(term->buffer, &term->buffer[term->buffSize], (size_t)term->buffSize); // term->buffSize / 2 * 2, will always be even number, so won't cause problems

		for(int i=term->buffSize; i<term->buffSize*2; i+=2)
		{
			term->buffer[i] = ' ';
			term->buffer[i+1] = term->color;
		}	

		term->cursorY -= term->buffSize / term->width / 2;

		term->displayY -= term->buffSize / term->width / 2;
		term->displayY++;
		term->lastCharacterY -= term->buffSize / term->width / 2;

		term->displayUpdated = true;
	}
}

static void next_line( Terminal *term)
{
	term->cursorY++;
	term->cursorX = 0;

	term->lastCharacterY = term->cursorY;
	check_last_character_y(term);
}

static void put_character( Terminal *term, char ch)
{
	switch(ch)
		{
			case '\n':
			{
				next_line(term);
				break;
			}
			
			case '\r':
			{
				next_line(term);
				break;
			}
		
			case '\t':
			{
				for(int i=0; i<4; i++)
					put_character(term, ' ');
				break;
			}
			
			default:
			{
				int pos = (term->cursorY * term->width + term->cursorX)*2;
				
				term->buffer[pos] = ch;
				term->buffer[pos+1] = term->color;

				term->cursorX++;
				if(term->cursorX >= term->width)
					next_line(term);
			}
		}
}

static void put_text( Terminal *term, char *str )
{
	while(*str != '\0')
	{
		put_character(term, *str);
		++str;
	}
}

static void print_uint( Terminal *term, unsigned int d )
{
	if(d==0)
	{
		put_character(term, '0');
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
	put_text(term, p);
}

static void print_int( Terminal *term, int d )
{	
	if(d<0)
	{
		d=-d;
		put_character(term, '-');
	}
	
	print_uint(term, (unsigned int) d);
}

static void print_hex( Terminal *term, unsigned int d ) //Seem to be fine but used to crush system
{
	put_text(term, "0x");
	
	if(d==0)
	{
		put_character(term, '0');
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
		put_character(term,  "0123456789abcdef"[d>>28]);
		d<<=4;
		++i;
	}
}

//___________________________________________________________________________________________________

void terminal_init( Terminal *term )
{
	term->buffSize = 4000; // in the future buffer will be dynamically allocated (it will be buffer size in bytes divided by 2)

	UI_manager_get_display_size(&term->width, &term->height);
	term->cursorX = 0;
	term->cursorY = 0;
	term->cursorEnabled = true;
	term->color = LIGH_GREEN;

	// clear buffer
	for(int i=0; i<term->buffSize*2; i+=2)
	{
		term->buffer[i] = ' ';
		term->buffer[i+1] = term->color;
	}

	term->displayY = 0;
	term->lastCharacterY = 0;

	terminal_print(term, "MATOS https://github.com/Mateusz1223/MatOS\nMateusz Piasecki https://piaseckimateusz.pl/\n\nWelcome to Terminal !!\n\n");
	term->displayUpdated = true;
}

void terminal_print( Terminal *term, const char *str, ... ) // May not work properly. May mix order
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
					print_int(term, va_arg(args, int));
					break;
				case 'x':
					print_hex(term, va_arg(args, int));
					break;
				case 'u':
					print_uint(term, va_arg(args, unsigned int));
					break;
				case 's':
					put_text(term, va_arg(args, char*));
					break;
				case '%':
					put_character(term, '%');
					break;
				default:
					put_character(term, *str);
					break;
			}
		}else put_character(term, *str);
		
		++str;
	}
	
	va_end(args);

	term->displayUpdated = true;
}

void terminal_putchar( Terminal *term, char ch )
{
	char str[2];
	str[0] = ch;
	str[1] = 0;
	terminal_print(term, str);
}

void terminal_set_color( Terminal *term, char ch )
{
	term->color = ch;

	int pos = (term->cursorY * term->width + term->cursorX)*2 + 1;
	term->buffer[pos] = ch;
}

void terminal_scan( Terminal *term, const char *str, ... )
{
	// TODO
}

void terminal_scroll_up( Terminal *term )
{
	term->displayY--;
	if(term->displayY < 0)
		term->displayY = 0;

	term->displayUpdated = true;
}

void terminal_scroll_down(Terminal *term)
{
	term->displayY++;
	if(term->displayY + term->height >= term->buffSize / term->width)
		term->displayY--;

	term->displayUpdated = true;
}