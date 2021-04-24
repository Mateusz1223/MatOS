#include "inc/drivers/screen.h"

#include "inc/HAL.h"

static struct CursorStruct
{
	int x;
	int y;
	uint16_t VGA_IO_Port_RegisterIndex;
	uint16_t VGA_IO_Port_DataRegister;
	int enabled;
}Cursor;

static struct ScreenStruct
{
	int height;
	int width;
	unsigned char color;
	unsigned char *textram;
}Screen;

//___________________________________________________________________________________________________

static void scroll_page();

static void cursor_new_line()
{
	int y = Cursor.y + 1;

	if(y >= Screen.height)
		scroll_page();
	else
	{
		Cursor.x = 0;
		Cursor.y = y;
	}
}

static void cursor_next_position()
{
	int x = Cursor.x + 1;

	if(x >= Screen.width)
		cursor_new_line();
	else
		Cursor.x = x;
}


static void cursor_get_position( int *x, int *y)
{
	*x = Cursor.x;
	*y = Cursor.y;
}

static void cursor_update()
{
	uint16_t pos = Cursor.y * Screen.width + Cursor.x;
	
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0xf);
	outb(Cursor.VGA_IO_Port_DataRegister, (uint8_t)(pos & 0xff));
	
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0xe);
	outb(Cursor.VGA_IO_Port_DataRegister, (uint8_t)((pos >> 8) & 0xff));
	
	if(Cursor.enabled)
		Screen.textram[(Cursor.y * Screen.width + Cursor.x)*2 + 1] = Screen.color;
}

//___________________________________________________________________________________________________

static void scroll_page()
{
	int max_pos = Screen.height*Screen.width*2;

	int start_copy = Screen.height/2;
	start_copy *= Screen.width*2;

	memmove(Screen.textram, &Screen.textram[start_copy], max_pos - start_copy);

	for(int i=max_pos-start_copy; i<max_pos; i+=2)
	{
		Screen.textram[i] = ' ';
		Screen.textram[i+1] = BACKGROUND_BLACK;
	}

	Cursor.x = 0;
	Cursor.y = (max_pos-start_copy)/(Screen.width*2);
	cursor_update();
}

//___________________________________________________________________________________________________

static void put_character(char ch)
{
	switch(ch)
		{
			case '\n':
			{
				int x, y;
				cursor_get_position(&x, &y);
				cursor_new_line();
				break;
			}
			
			case '\r':
			{
				int x, y;
				cursor_get_position(&x, &y);
				cursor_new_line();
				break;
			}
		
			case '\t':
			{
				for(int i=0; i<4; i++)
				{
					int x, y;
					cursor_get_position(&x, &y);
					int pos = (y * Screen.width + x)*2;
					
					Screen.textram[pos] = ' ';
					Screen.textram[pos+1] = Screen.color;

					cursor_next_position();
				}
				break;
			}
			
			default:
			{
				int x, y;
				cursor_get_position(&x, &y);
				int pos = (y * Screen.width + x)*2;
				
				Screen.textram[pos] = ch;
				Screen.textram[pos+1] = Screen.color;

				cursor_next_position();
			}
		}
}

static void put_text( char *str )
{
	while(*str != '\0')
	{
		put_character(*str);
		++str;
	}
}

static void print_uint( unsigned int d )
{
	if(d==0)
	{
		put_character('0');
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
	put_text(p);
}

static void print_int( int d )
{	
	if(d<0)
	{
		d=-d;
		put_character('-');
	}
	
	print_uint( (unsigned int) d);
}

static void print_hex( unsigned int d ) //Seem to be fine but used to crush system
{
	put_text("0x");
	
	if(d==0)
	{
		put_character('0');
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
		put_character( "0123456789abcdef"[d>>28]);
		d<<=4;
		++i;
	}
}

//___________________________________________________________________________________________________

void screen_clear()
{
	int n=Screen.height*Screen.width*2;
	int i;
	for(i=0; i<n; i+=2)
	{
		Screen.textram[i] = ' ';
		Screen.textram[i+1] = BACKGROUND_BLACK;
	}
	Cursor.x = 0;
	Cursor.y = 0;
	cursor_update();
}

void screen_putchar( char ch )
{
	char *str;
	str[0] = ch;
	str[1] = '\0';
	screen_print(str);
}

void screen_print( const char *str, ... ) // Doesn't work propertly. To be fixed (Mixes order witch few arguments eg. "%x %x %x" in memory_menager.c) 
{
	va_list args;
	va_start(args, str);
	
	while(*str != '\0')
	{
		if(*str=='%')
		{
			++str;
			switch(*str)
			{
				case 'i':
				case 'd':
					print_int(va_arg(args, int));
					break;
				case 'x':
					print_hex(va_arg(args, int));
					break;
				case 'u':
					print_uint(va_arg(args, unsigned int));
					break;
				case 's':
					put_text(va_arg(args, char*));
					break;
				case '%':
					put_character('%');
					break;
				default:
					put_character(*str);
					break;
			}
		}else put_character(*str);
		
		++str;
	}
	
	va_end(args);

	cursor_update();
}

void screen_set_color( char ch)
{
	Screen.color = ch;
	int pos = (Cursor.y * Screen.width + Cursor.x)*2 + 1;
	Screen.textram[pos] = ch;
}

void screen_disable_cursor()
{
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0x0A);
	outb(Cursor.VGA_IO_Port_DataRegister, 0x20);
	
	Cursor.enabled = 0;

	Screen.textram[(Cursor.y * Screen.width + Cursor.x)*2 + 1] = BLACK;
}

void screen_enable_cursor()
{
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0x0A);
	outb(Cursor.VGA_IO_Port_DataRegister, (inb(0x3D5) & 0xC0) | 14);
 
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0x0B);
	outb(Cursor.VGA_IO_Port_DataRegister, (inb(0x3E0) & 0xE0) | 17);
	
	Cursor.enabled = 1;

	Screen.textram[(Cursor.y * Screen.width + Cursor.x)*2 + 1] = Screen.color;
}

void screen_init(bootinfo* boot_info)
{
	Screen.height = 25;
	if((boot_info->BIOS_equipment_list & 0x30)>>4 == 0x2)
		Screen.width = 80;
	else if((boot_info->BIOS_equipment_list & 0x30)>>4 == 0x1)
		Screen.width = 40;

	Cursor.x = 0;
	Cursor.y = 0;
	Screen.color = LIGH_GREEN;
	Screen.textram = (unsigned char *)0xb8000;
	Cursor.VGA_IO_Port_RegisterIndex = 0x3d4;
	Cursor.VGA_IO_Port_DataRegister = 0x3d5;
	Cursor.enabled = 1;

	screen_clear();
	screen_print("Screen initialized!\n");
}