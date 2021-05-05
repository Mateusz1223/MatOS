#include "inc/drivers/keyboard.h"

#include "inc/HAL.h"
#include "inc/drivers/PIC.h"
#include "inc/UI/UIManager.h"

#define TASK_NONEOPERATION 0x0
#define TASK_GETRESPONSE 0x1
#define TASK_SCAN 0x2

static struct KeyboardState
{
	bool pressedKeys[76]; // 1->75, 0 is undefined key, [id]

	bool capsLockActive;
} Keyboard;

typedef struct KeyboardAction
{
	int id;
	bool released; // false -> key pressed, true -> key released
} KeyboardAction;

// __________________________________________________________________________

void disable_keyboard_irq()
{
	IRQ_set_mask(1);
}

void enable_keyboard_irq()
{
	IRQ_clear_mask(1);
}

char getScancode()
{
	while(1)
	{
		unsigned char c = inb(0x60);
		if(c != 0)
			return c;
	}
}

// __________________________________________________________________________

void keyboard_init()
{
	Keyboard.capsLockActive = 0;

	enable_keyboard_irq();

	terminal_print(debugTerminal, "PS/2 Keyboard ready!\n");
}

bool previous_E0 = false;

int to_ignore = 0;

void keyboard_irq()
{
	unsigned char ch = getScancode();

	// debug
	/*screen_set_color(RED);
	terminal_print(debugTerminal, "Keybord interrupt. Scancode: %x\n", ch);
	screen_set_color(LIGHT_GREEN);*/

	if(to_ignore > 0)
	{
		to_ignore--;
		return;
	}

	if(ch == 0xE1)
	{
		to_ignore = 2;
		return;
	}
	
	if(ch == 0xE0)
	{
		previous_E0 = true;
		return;
	}

	KeyboardAction action;

	action.id = 0; // undefined key
	action.released = true;

	if(previous_E0)
	{
		previous_E0 = false;
		switch(ch)
	    {
	    // Pressed
	    case 0x1C:
	        // Enter
	    	action.id = 55;
	    	action.released = false;
	        break;
	    case 0x38:
	        // Right Alt
	    	action.id = 71;
	    	action.released = false;
	        break;
	    case 0x48:
	        // Cursor Up
	    	action.id = 73;
	    	action.released = false;
	        break;
	    case 0x4b:
	        // Cursor Left
	    	action.id = 72;
	    	action.released = false;
	        break;
	    case 0x4d:
	        // Cursor Right
	    	action.id = 75;
	    	action.released = false;
	        break;
	    case 0x50:
	        // Cursor Down
	    	action.id = 74;
	    	action.released = false;
	        break;
	    case 0x53:
	        // Delete
	    	action.id = 14;
	    	action.released = false;
	        break;

	    // Released
	    case 0x9C:
	        // Enter
	    	action.id = 55;
	    	action.released = true;
	        break;
	    case 0xB8:
	        // Right Alt
	    	action.id = 71;
	    	action.released = true;
	        break;
	    case 0xC8:
	        // Cursor Up
	    	action.id = 73;
	    	action.released = true;
	        break;
	    case 0xCB:
	        // Cursor Left
	    	action.id = 72;
	    	action.released = true;
	        break;
	    case 0xCD:
	        // Cursor Right
	    	action.id = 75;
	    	action.released = true;
	        break;
	    case 0xD0:
	        // Cursor Down
	    	action.id = 74;
	    	action.released = true;
	        break;
	    case 0xD3:
	        // Delete
	    	action.id = 14;
	    	action.released = true;
	        break;

	    default:
	    	action.id = 0;
	    	action.released = true;
	    	break;
	    }
	}
	else
	{	
		// Presses

		if(0x3B <= ch && ch <= 0x44) // F1 -> F10
		{
			action.id = ch-57; // 2 -> 11
			action.released = false;
		}
		else if(0x57 <= ch && ch <= 0x58) // F11 -> F12
		{
			action.id = ch-75; // 12 -> 13
			action.released = false;
		}
		else if(0x02 <= ch && ch <= 0x1B) // 1 -> ]
		{
			action.id = ch+14; // 16 -> 41
			action.released = false;
		}
		else if(0x1E <= ch && ch <= 0x28) // A -> '
		{
			action.id = ch+14; // 44 -> 54
			action.released = false;
		}
		else if(0x2C <= ch && ch <= 0x36) // Z -> Right Shift
		{
			action.id = ch+13; // 57 -> 67
			action.released = false;
		}
		else if(ch == 0x1) // Escape
		{
			action.id = 1; // 1
			action.released = false;
		}
		else if(ch == 0x29) // ` (~)
		{
			action.id = 15; // 15
			action.released = false;
		}
		else if(ch == 0x2B) // '\'
		{
			action.id = 42; // 42
			action.released = false;
		}
		else if(ch == 0x3A) // Caps lock
		{
			action.id = 43; // 43
			action.released = false;

			if(Keyboard.capsLockActive == 0)
				Keyboard.capsLockActive = 1;
			else
				Keyboard.capsLockActive = 0;
		}
		else if(ch == 0x1C) // Enter
		{
			action.id = 55; // 55
			action.released = false;
		}
		else if(ch == 0x2A) // Left Shift
		{
			action.id = 56; // 56
			action.released = false;
		}
		else if(ch == 0x1D) // Left Control
		{
			action.id = 68; // 68
			action.released = false;
		}
		else if(ch == 0x38) // Left Alt
		{
			action.id = 69; // 69
			action.released = false;
		}
		else if(ch == 0x39) // Space
		{
			action.id = 70; // 70
			action.released = false;
		}

		// Releaseds

		else if(0xBB <= ch && ch <= 0xC4) // F1 -> F10
		{
			action.id = ch-185; // 2 -> 11
			action.released = true;
		}
		else if(0xD7 <= ch && ch <= 0xD8) // F11 -> F12
		{
			action.id = ch-203; // 12 -> 13
			action.released = true;
		}
		else if(0x82 <= ch && ch <= 0x9B) // 1 -> ]
		{
			action.id = ch-114; // 16 -> 41
			action.released = true;
		}
		else if(0x9E <= ch && ch <= 0xA8) // A -> '
		{
			action.id = ch-114; // 44 -> 54
			action.released = true;
		}
		else if(0xAC <= ch && ch <= 0xB6) // Z -> Right Shift
		{
			action.id = ch-115; // 57 -> 67
			action.released = true;
		}
		else if(ch == 0x81) // Escape
		{
			action.id = 1; // 1
			action.released = true;
		}
		else if(ch == 0xA9) // ` (~)
		{
			action.id = 15; // 15
			action.released = true;
		}
		else if(ch == 0xAB) // '/'
		{
			action.id = 42; // 42
			action.released = true;
		}
		else if(ch == 0xBA) // Caps lock
		{
			action.id = 43; // 43
			action.released = true;
		}
		else if(ch == 0x9C) // Enter
		{
			action.id = 55; // 55
			action.released = true;
		}
		else if(ch == 0xAA) // Left Shift
		{
			action.id = 56; // 56
			action.released = true;
		}
		else if(ch == 0x9D) // Left Control
		{
			action.id = 68; // 68
			action.released = true;
		}
		else if(ch == 0xB8) // Left Alt
		{
			action.id = 69; // 69
			action.released = true;
		}
		else if(ch == 0xB9) // Space
		{
			action.id = 70; // 70
			action.released = true;
		}
	}

	if(action.released == false)
	{
		Keyboard.pressedKeys[action.id] = true;

		UI_manager_keyboard_irq_resident(action.id);
	}
	else
	{
		Keyboard.pressedKeys[action.id] = false;
	}
}

char keyIdLookUpTable[] = // [id], -5 - cursor right, -4 - cursor left, -3 - enter, -2 - maybe to be added, -1 - undefined, 8 -> backspace, 127 - delete,
{
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	127, // DEL
	'`',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0',
	'-',
	'=',
	8,
	'\t',
	'q',
	'w',
	'e',
	'r',
	't',
	'y',
	'u',
	'i',
	'o',
	'p',
	'[',
	']',
	'\\',
	-1,
	'a',
	's',
	'd',
	'f',
	'g',
	'h',
	'j',
	'k',
	'l',
	';',
	'\'',
	-3,
	-1,
	'z',
	'x',
	'c',
	'v',
	'b',
	'n',
	'm',
	',',
	'.',
	'/',
	-1,
	-1,
	-1,
	' ',
	-1,
	-4,
	-1,
	-1,
	-5,
};

/*
	IDs


	ID  KEY
	___________________
	1	ESCAPE
	2	F1
	3	F2
	4	F3
	5	F4
	6	F5
	7	F6
	8	F7
	9	F8
	10	F9
	11	F10
	12	F11
	13	F12
	14	Delete
	15	`'
	16	1
	17	2
	18	3
	19	4
	20	5
	21	6
	22	7
	23	8
	24	9
	25	0
	26	-
	27	=
	28	Backspace
	29	Tab
	30	q
	31	w
	32	e
	33	r
	34	t
	35	y
	36	u
	37	i
	38	o
	39	p
	40	[
	41	]
	42	\
	43	Caps lock
	44	a
	45	s
	46	d
	47	f
	48	g
	49	h
	50	j
	51	k
	52	l
	53	;
	54	''
	55	ENTER
	56	Left Shift
	57	z
	58	x
	59	c
	60	v
	61	Backspace
	62	n
	63	m
	64	,
	65	.
	66	/
	67	Right Shift
	68	Left Control
	69	Left Alt
	70	Space
	71	Right Alt
	72	Cursor Left
	73	Cursor Up
	74	Cursor Down
	75	Cursor Right

*/