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

static void put_character(char ch)
{
	switch(ch)
		{
			if(Cursor.enabled != 0)
				{
					case '\n':
					{
						int x, y;
						screen_get_cursor_position(&x, &y);
						y+=1;
						Cursor.x=0;
						Cursor.y=y;
						break;
					}
					
					case '\r':
					{
						int x, y;
						screen_get_cursor_position(&x, &y);
						Cursor.x=0;
						Cursor.y=y;
						break;
					}
				}
			
			case '\t':
			{
				if(Cursor.enabled != 0)
				{
					int x, y;
					screen_get_cursor_position(&x, &y);
					
					if(x>76)
					{
						y+=1;
						Cursor.x=0;
						Cursor.y=y;
					}
					else
					{
						x+=4;
						Cursor.x=x;
						Cursor.y=y;
					}
					break;
				}
			}
			
			default:
				if(Cursor.enabled != 0)
				{
					int pos = (Cursor.y * Screen.width + Cursor.x)*2;
					int new_x = Cursor.x+1;
					int new_y = Cursor.y;
					if(new_x == Screen.width)
					{
						new_x = 0;
						new_y += 1;
					}
					
					Screen.textram[pos] = ch;
					Screen.textram[pos+1] = Screen.color;

					Cursor.x=new_x;
					Cursor.y=new_y;
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

void screen_set_cursor_position( int x, int y)
{
	if(x < Screen.width && y < Screen.height)
	{
		uint16_t pos = y * Screen.width + x;
		
		outb(Cursor.VGA_IO_Port_RegisterIndex, 0xf);
		outb(Cursor.VGA_IO_Port_DataRegister, (uint8_t)(pos & 0xff));
		
		outb(Cursor.VGA_IO_Port_RegisterIndex, 0xe);
		outb(Cursor.VGA_IO_Port_DataRegister, (uint8_t)((pos >> 8) & 0xff));
		
		Cursor.x=x;
		Cursor.y=y;
		
		Screen.textram[(y * Screen.width + x)*2 + 1] = Screen.color;
	}
	else if(x < Screen.width && y >= Screen.height)
	{
		screen_clear();
	}
}

void screen_get_cursor_position( int *x, int *y)
{
	*x = Cursor.x;
	*y = Cursor.y;
}

void screen_clear()
{
	int n=Screen.height*Screen.width*2;
	int i;
	for(i=0; i<n; i+=2)
	{
		Screen.textram[i]=' ';
	}
	screen_set_cursor_position(0, 0);
}

void screen_putchar( char ch )
{
	char *str;
	str[0] = ch;
	str[1] = '\0';
	screen_print(str);
}

void screen_print( const char *str, ... )//Doesn't work propertly. To be fixed (Mixes order witch few arguments eg. "%x %x %x" in memory_menager.c) 
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

	screen_set_cursor_position(Cursor.x, Cursor.y);
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
}

void screen_enable_cursor()
{
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0x0A);
	outb(Cursor.VGA_IO_Port_DataRegister, (inb(0x3D5) & 0xC0) | 14);
 
	outb(Cursor.VGA_IO_Port_RegisterIndex, 0x0B);
	outb(Cursor.VGA_IO_Port_DataRegister, (inb(0x3E0) & 0xE0) | 17);
	
	Cursor.enabled = 1;
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