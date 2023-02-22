#include "inc/drivers/busses/ATA.h"

#include "inc/HAL.h"
#include "inc/UI/terminal.h"

// https://wiki.osdev.org/PCI_IDE_Controller

// Registers offset
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D
/*
----------------------REG info----------------------

BAR0 is the start of the I/O ports used by the primary channel.
BAR1 is the start of the I/O ports which control the primary channel.
BAR2 is the start of the I/O ports used by secondary channel.
BAR3 is the start of the I/O ports which control secondary channel.
BAR4 is the start of 8 I/O ports controls the primary channel's Bus Master IDE.
BAR4 + 8 is the Base of 8 I/O ports controls secondary channel's Bus Master IDE.

The ALTSTATUS/CONTROL port returns the alternate status when read and controls a channel when written to.

For the primary channel, ALTSTATUS/CONTROL port is BAR1 + 2.
For the secondary channel, ALTSTATUS/CONTROL port is BAR3 + 2.
We can now say that each channel has 13 registers. For the primary channel, we use these values:

- Data Register: BAR0 + 0; // Read-Write
- Error Register: BAR0 + 1; // Read Only
- Features Register: BAR0 + 1; // Write Only
- SECCOUNT0: BAR0 + 2; // Read-Write
- LBA0: BAR0 + 3; // Read-Write
- LBA1: BAR0 + 4; // Read-Write
- LBA2: BAR0 + 5; // Read-Write
- HDDEVSEL: BAR0 + 6; // Read-Write, used to select a drive in the channel.
- Command Register: BAR0 + 7; // Write Only.
- Status Register: BAR0 + 7; // Read Only.
- Alternate Status Register: BAR1 + 2; // Read Only.
- Control Register: BAR1 + 2; // Write Only.
- DEVADDRESS: BAR1 + 3; // I don't know what is the benefit from this register.

The map above is the same with the secondary channel, but it uses BAR2 and BAR3 instead of BAR0 and BAR1.
*/

// I assume that all BARs are equal to 0

// The Command/Status Port returns a bit mask referring to the status of a channel when read:
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0 
#define ATA_CMD_IDENTIFY_PACKET   0xA1 // returns a buffer of 512 bytes called the identification space
#define ATA_CMD_IDENTIFY          0xEC // returns a buffer of 512 bytes called the identification space

// the following definitions are used to read information from the identification space:
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
#define IDE_UNKNOWN    0x02

struct IDEChannelRegisters {
   unsigned short base;  // I/O Base.
   unsigned short ctrl;  // Control Base
   unsigned short bmide; // Bus Master IDE
   unsigned char  nIEN;  // nIEN (No Interrupt);
} channels[2];

unsigned char ide_buf[2048] = {0};
volatile unsigned static char ide_irq_invoked = 0;
unsigned static char atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct ide_device {
   unsigned char  Reserved;    // 0 (Empty) or 1 (This Drive really exists).
   unsigned char  Channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
   unsigned char  Drive;       // 0 (Master Drive) or 1 (Slave Drive).
   unsigned short Type;        // 0: ATA, 1:ATAPI.
   unsigned short Signature;   // Drive Signature
   unsigned short Capabilities;// Features.
   unsigned int   CommandSets; // Command Sets Supported.
   unsigned int   Size;        // Size in Sectors.
   unsigned char  Model[41];   // Model in string.
} ide_devices[4];

//___________________________________________________________________________________________________

static void ide_write(unsigned char channel, unsigned char reg, unsigned char data) {
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      outb(channels[channel].base  + reg - 0x00, data);
   else if (reg < 0x0C)
      outb(channels[channel].base  + reg - 0x06, data);
   else if (reg < 0x0E)
      outb(channels[channel].ctrl  + reg - 0x0A, data);
   else if (reg < 0x16)
      outb(channels[channel].bmide + reg - 0x0E, data);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

static unsigned char ide_read(unsigned char channel, unsigned char reg) {
   unsigned char result;
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      result = inb(channels[channel].base + reg - 0x00);
   else if (reg < 0x0C)
      result = inb(channels[channel].base  + reg - 0x06);
   else if (reg < 0x0E)
      result = inb(channels[channel].ctrl  + reg - 0x0A);
   else if (reg < 0x16)
      result = inb(channels[channel].bmide + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
   return result;
}

static void ide_read_buffer(unsigned char channel, unsigned char reg, unsigned int *buffer,
                     unsigned int quads) {
   /* WARNING: This code contains a serious bug. The inline assembly trashes ES and
    *           ESP for all of the code the compiler generates between the inline
    *           assembly blocks.
    */
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   //asm("pushw %es; movw %ds, %ax; movw %ax, %es");
   if (reg < 0x08)
      insd(channels[channel].base  + reg - 0x00, buffer, quads);
   else if (reg < 0x0C)
      insd(channels[channel].base  + reg - 0x06, buffer, quads);
   else if (reg < 0x0E)
      insd(channels[channel].ctrl  + reg - 0x0A, buffer, quads);
   else if (reg < 0x16)
      insd(channels[channel].bmide + reg - 0x0E, buffer, quads);
   //asm("popw %es;");
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

//___________________________________________________________________________________________________

void ATA_init(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4){
   int i, j, k, count = 0;
 
   // 1- Detect I/O Ports which interface IDE Controller:
   channels[ATA_PRIMARY  ].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
   channels[ATA_PRIMARY  ].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
   channels[ATA_SECONDARY].base  = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
   channels[ATA_SECONDARY].ctrl  = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
   channels[ATA_PRIMARY  ].bmide = (BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
   channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE
   // 2- Disable IRQs:
   ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
   ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
   // 3- Detect ATA-ATAPI Devices:
   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++){
         unsigned char err = 0, type = IDE_ATA, status;
         ide_devices[count].Reserved = 0; // Assuming that no drive here.
 
         // (I) Select Drive:
         ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
         io_wait(); // Wait 1ms for drive select to work.
 
         // (II) Send ATA Identify Command:
         ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
         io_wait(); // This function should be implemented in your OS. which waits for 1 ms.
                   // it is based on System Timer Device Driver.
 
         // (III) Polling:
         if (ide_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.
 
         while(1) {
            status = ide_read(i, ATA_REG_STATUS);
            if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
            if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
         }
 
         // (IV) Probe for ATAPI Devices:
 
         if (err != 0){
            unsigned char cl = ide_read(i, ATA_REG_LBA1);
            unsigned char ch = ide_read(i, ATA_REG_LBA2);
 
            if (cl == 0x14 && ch == 0xEB)
               type = IDE_ATAPI;
            else if (cl == 0x69 && ch == 0x96)
               type = IDE_ATAPI;
            else
               continue; // Unknown Type (may not be a device).
 
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            io_wait();
         }
 
         // (V) Read Identification Space of the Device:
         ide_read_buffer(i, ATA_REG_DATA, (unsigned int *)ide_buf, 128);
 
         // (VI) Read Device Parameters:
         ide_devices[count].Reserved     = 1;
         ide_devices[count].Type         = type;
         ide_devices[count].Channel      = i;
         ide_devices[count].Drive        = j;
         ide_devices[count].Signature    = *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE));
         ide_devices[count].Capabilities = *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES));
         ide_devices[count].CommandSets  = *((unsigned int *)(ide_buf + ATA_IDENT_COMMANDSETS));
 
         // (VII) Get Size:
         if (ide_devices[count].CommandSets & (1 << 26))
            // Device uses 48-Bit Addressing:
            ide_devices[count].Size   = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
         else
            // Device uses CHS or 28-bit Addressing:
            ide_devices[count].Size   = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA));
 
         // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
         for(k = 0; k < 40; k += 2) {
            ide_devices[count].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
            ide_devices[count].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];}
         ide_devices[count].Model[40] = 0; // Terminate String.
 
         count++;
      }
 
   // 4- Print Summary:
   for (i = 0; i < 4; i++)
      if (ide_devices[i].Reserved == 1) {
         terminal_print(debugTerminal, " Found %s Drive %dMB - %s\n",
            (const char *[]){"ATA", "ATAPI"}[ide_devices[i].Type],         /* Type */
            ide_devices[i].Size / 1024 / 2,               /* Size */
            ide_devices[i].Model);
      }
}


void ATA_check(){
	terminal_print(debugTerminal, "ATA check:\n");

	outb(0x1F0 + ATA_REG_HDDEVSEL, 0xA0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait();

	// what about identify command ??

	uint8_t tmpword = inb(0x1F0 + ATA_REG_STATUS); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
		terminal_print(debugTerminal, "Primary master exists\n");
	else
		terminal_print(debugTerminal, "Primary master does not exist\n");

	outb(0x1F0 + ATA_REG_HDDEVSEL, (0xA0) | (1 << 4)); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait();

	// what about identify command ??

	tmpword = inb(0x1F0 + ATA_REG_STATUS); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
		terminal_print(debugTerminal, "Primary slave exists\n");
	else
		terminal_print(debugTerminal, "Primary slave does not exist\n");

	outb(0x170 + ATA_REG_HDDEVSEL, 0xA0); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	tmpword = inb(0x170 + ATA_REG_STATUS); // read the status port

	// what about identify command ??

	if (tmpword & 0x40) // see if the busy bit is set
		terminal_print(debugTerminal, "Secondary master exists\n");
	else
		terminal_print(debugTerminal, "Secondary master does not exist\n");

	outb(0x170 + ATA_REG_HDDEVSEL, (0xA0) | (1 << 4)); // use 0xB0 instead of 0xA0 to test the second drive on the controller

	io_wait(); // wait 1/250th of a second

	// what about identify command ??

	tmpword = inb(0x170 + ATA_REG_STATUS); // read the status port

	if (tmpword & 0x40) // see if the busy bit is set
		terminal_print(debugTerminal, "Secondary slave exists\n");
	else
		terminal_print(debugTerminal, "Secondary slave does not exist\n\n");
}