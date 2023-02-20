#include "inc/drivers/timers/RTC.h"

#include "inc/HAL.h"
#include "inc/interrupts/interrupts.h"
#include "inc/drivers/PIC.h"

#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

static struct RTCStruct
{
	int second;
	int minute;
	int hour;

	int rate; // 3 to 15

	int intAmt; // used to determine when to increment second
}RTC;

//_________________________________________________________________________________________

static void disable_RTC_irq()
{
	IRQ_set_mask(8);
}

static void enable_RTC_irq()
{
	IRQ_clear_mask(8);
}

static unsigned char get_RTC_register(int reg)
{
      outb(0x70, (1 << 7) | reg);
      io_wait();
      return inb(0x71);
}

static bool update_in_progress()
{
	return (bool)((get_RTC_register(0x0A) << 1) >> 7);
}

//_________________________________________________________________________________________

void RTC_init()
{
	uint8_t s;
	uint8_t m;
	uint8_t h;

	disable_interrupts();

	// Read from CMOS
	
	/*// may take up to 1 second, prevents from reading dodgy data for example minute: 60
	while(!update_in_progress());
	while(update_in_progress());

	m = get_RTC_register(0x02);
	h = get_RTC_register(0x04);*/

	// Prevents from reading dodgy data for example minute: 60
	do {
		while (update_in_progress());
		s = get_RTC_register(0x00);
		m = get_RTC_register(0x02);
		h = get_RTC_register(0x04);
	} while (update_in_progress());

	uint8_t registerB = get_RTC_register(0x0B);
 
    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04))
    {
    	s = (s & 0x0F) + ((s / 16) * 10);
        m = (m & 0x0F) + ((m / 16) * 10);
        h = ( (h & 0x0F) + (((h & 0x70) / 16) * 10) ) | (h & 0x80);
    }

    // Convert 12 hour clock to 24 hour clock if necessarys
    if (!(registerB & 0x02) && (h & 0x80)) {
            h = ((h & 0x7F) + 12) % 24;
      }

    RTC.second = (int)s;
    RTC.minute = (int)m;
	RTC.hour = (int)h;

	if(RTC.hour == 24)
		RTC.hour = 0;

	UI_manager_RTC_irq_resident(RTC.hour, RTC.minute); // To set UI taskbar time

	// Set frequency
	RTC.rate = 15; // frequency =  32768 >> (rate-1);
	RTC.rate &= 0x0F;			// rate must be above 2 and not over 15
	uint8_t prev = get_RTC_register(0x0A);
	outb(0x70, (1 << 7) | (0x0A));		// reset index to A
	outb(0x71, (prev & 0xF0) | RTC.rate); //write only our rate to A. Note, rate is the bottom 4 bits.
	RTC.intAmt = 0;


	// Enable RTC interrupts
	prev = get_RTC_register(0x0B);
	outb(0x70, (1 << 7) | (0x0B));
	outb(0x71, prev | 0x40);

	enable_interrupts();
	enable_RTC_irq();

	outb(0x70, 0x0C);	// select register C
	io_wait();
	inb(0x71);		// just throw away contents

	terminal_print(debugTerminal, "[X] RTC ready, time: %d:%d:%d\n", RTC.hour, RTC.minute, RTC.second);
}

void RTC_get_time(int *h, int *m, int *s)
{
	*h = RTC.hour;
	*m = RTC.minute;
	*s = RTC.second;
}

void RTC_irq()
{
	//terminal_print(debugTerminal, "RTC interrupt\n");

	RTC.intAmt++;

	int frequency = 32768 >> (RTC.rate-1);

	if(RTC.intAmt >= frequency)
	{
		RTC.intAmt = 0;

		RTC.second++;
		if(RTC.second >= 60)
		{
			RTC.second = 0;

			RTC.minute++;
			if(RTC.minute >= 60)
			{
				RTC.minute = 0;

				RTC.hour++;
				if(RTC.hour == 24)
					RTC.hour = 0;
			}

			UI_manager_RTC_irq_resident(RTC.hour, RTC.minute);
		}
	}

	outb(0x70, 0x0C);	// select register C
	io_wait();
	inb(0x71);		// just throw away contents
}