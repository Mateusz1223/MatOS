#include "include/keyboard.h"

#define TASK_NONEOPERATION 0x0
#define TASK_GETRESPONSE 0x1
#define TASK_SCAN 0x2

int send_byte_to_keyboard(uint8_t byte)
{
	disable_keyboard_irq();
	task_answer = 0;
	
	outb(0x60, byte);
	task = TASK_GETRESPONSE;
	enable_keyboard_irq();
	
	while(!task_answer);
	disable_keyboard_irq();
	
	if(task_answer == 0xFE)
	{
		task_answer = 0;
		
		outb(0x60, byte);
		task = TASK_GETRESPONSE;
		enable_keyboard_irq();
		
		while(!task_answer);
		disable_keyboard_irq();
		
		if(task_answer == 0xFE)
		{
			task_answer = 0;
			
			outb(0x60, byte);
			task = TASK_GETRESPONSE;
			enable_keyboard_irq();
			
			while(!task_answer);
			disable_keyboard_irq();
			
			if(task_answer == 0xFE)
			{
				task_answer = 0;
				task = TASK_NONEOPERATION;
				return -1;
			}
		}
	}
	
	task_answer = 0;
	task = TASK_NONEOPERATION;
	return 0;
}

void disable_keyboard_irq()
{
	IRQ_set_mask(1);
}

void enable_keyboard_irq()
{
	IRQ_clear_mask(1);
}

void disable_keyboard_scanning()
{
	disable_keyboard_irq();
	
	if(send_byte_to_keyboard(0xF5) == -1)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return;
	}
	
	enable_keyboard_irq();
	return;
}

void enable_keyboard_scanning()
{
	disable_keyboard_irq();
	
	if(send_byte_to_keyboard(0xF4) == -1)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return;
	}
	
	enable_keyboard_irq();	
	return;
}

void keyboard_set_ScanCodeSet(uint8_t scan_code_set)
{
	disable_keyboard_scanning();
	disable_keyboard_irq();
	
	if(scan_code_set < 1 || scan_code_set > 3)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return;
	}
	
	if(send_byte_to_keyboard(0xF0) == -1)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return;
	}
		
	
	if(send_byte_to_keyboard(scan_code_set) == -1)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return;
	}
	
	task_answer = 0;
	task = TASK_NONEOPERATION;
	
	enable_keyboard_irq();	
	enable_keyboard_scanning();
	return;
}

int keyboard_get_ScanCodeSet()
{	
	disable_keyboard_scanning();

	if(send_byte_to_keyboard(0xF0) == -1)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return -1;
	}
		
	
	if(send_byte_to_keyboard(0) == -1)
	{
		enable_keyboard_irq();	
		enable_keyboard_scanning();
		return -1;
	}
	
	task_answer = 0;
	
	task = TASK_GETRESPONSE;
	enable_keyboard_irq();
	while(!task_answer);
	
	unsigned char ret = task_answer;
	
	task_answer = 0;
	task = TASK_NONEOPERATION;
	
	enable_keyboard_irq();
	enable_keyboard_scanning();
	return ret;
}

char str[20];
char *keyboard_scan_input()
{
	int i = 0;
	while(1)
	{
		task_answer = 0;
		task = TASK_SCAN;
		while(!task_answer);
		if(task_answer == 0xa)
		{
			str[i] = 0;
			task = TASK_NONEOPERATION;
			screen_putchar('\n');
			return str;
		}
		if(task_answer != 0)
		{
			screen_putchar(task_answer);
			str[i] = task_answer;
		}
		++i;
	}
	
}

char getScancode()
{
	//screen_print("Scan:\n");
	while(1)
	{
		unsigned char c = inb(0x60);
		if(c != 0)
		{
			//screen_print("%x\n", c);
			return c;
		}
	}
}

char ScancodeToASCII(char c)
{
	if(c == 15)
		return '\t';

	if(c == 57)
		return ' ';

	if(c == 14)
		return '\b';
	
	if(c == 28)
		return 0xa;
	
	if(Shift == 1)
	{
		if(c >= 2 && c <= 13)
		{
			return "!@#$%^&*()_+"[c - 2];
		}
		else if(c == 26 || c == 27)
		{
			return "{}"[c - 26];
		}
		else if(c >= 39 && c <= 41)
		{
			return ":\"~"[c - 39];
		}
		else if(c == 43)
		{
			return '|';
		}
		else if(c >= 51 && c <= 53)
		{
			return "<>?"[c - 51];
		}
	}
	else if(Shift == 0)
	{
		if(c >= 2 && c <= 13)
		{
			return "1234567890-="[c - 2];
		}
		else if(c == 26 || c == 27)
		{
			return "[]"[c - 26];
		}
		else if(c >= 39 && c <= 41)
		{
			return ";'`"[c - 39];
		}
		else if(c == 43)
		{
			return '\\';
		}
		else if(c >= 51 && c <= 53)
		{
			return ",./"[c - 51];
		}
	}
	if((CapsLock == 1 && Shift ==0) || (CapsLock == 0 && Shift ==1))
	{
		if(c >= 16 && c <= 25)
		{
			return "QWERTYUIOP"[c - 16];
		}
		else if(c >= 30 && c <= 38)
		{
			return "ASDFGHJKL"[c - 30];
		}
		else if(c >= 44 && c <= 50)
		{
			return "ZXCVBNM"[c - 44];
		}
	}
	else if((CapsLock == 0 && Shift ==0) || (CapsLock == 1 && Shift == 1))
	{
		if(c >= 16 && c <= 25)
		{
			return "qwertyuiop"[c - 16];
		}
		else if(c >= 30 && c <= 38)
		{
			return "asdfghjkl"[c - 30];
		}
		else if(c >= 44 && c <= 50)
		{
			return "zxcvbnm"[c - 44];
		}
	}
	return 0;
}

void keyboard_driver()
{
	if(task == TASK_NONEOPERATION)
	{
		char c = getScancode();
		if(c == 0x3a)
			CapsLock =! CapsLock;
		else if(c == 0x45)
			NumLock =! NumLock;
		return;
	}
	if(task == TASK_GETRESPONSE)
	{
		task_answer = getScancode();
		return;
	}
	if(task == TASK_SCAN)
	{
		unsigned char c = getScancode();
		
		if(c == 0x3a)
			CapsLock =! CapsLock;
		else if(c == 0x45)
			NumLock =! NumLock;
		else if(c == 0x2A || c == 0x36)
			Shift = 1;
		else if(c == 0xAA || c == 0xB6)
			Shift = 0;
		else
			task_answer = ScancodeToASCII(c);
		
		return;
	}
}








