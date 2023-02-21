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
	uint16_t base;  // I/O Base.
	uint16_t ctrlBase;  // Control Base
	uint16_t busMasterIDE; // Bus Master IDE
	uint8_t  noInt;  // noInt (No Interrupt);

} channels[2];

struct ideDevice {
	bool  exists;    // 0 (Empty) or 1 (This Drive really exists).
	uint8_t channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
	uint8_t drive;       // 0 (Master Drive) or 1 (Slave Drive).
	uint16_t type;        // 0: ATA, 1:ATAPI.
	uint16_t signature;   // Drive Signature
	uint16_t capabilities;// Features.
	unsigned int commandSets; // Command Sets Supported.
	unsigned int size;        // Size in Sectors.
	char model[41];   // Model in string.

} ideDevices[4];

//___________________________________________________________________________________________________

//___________________________________________________________________________________________________

void ATA_write(uint8_t channel, uint8_t reg, uint8_t data){
	// to understand this reg translation see REG info
	if (reg > 0x07 && reg < 0x0C) 
		ATA_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].noInt); // not sure what it does
	if (reg < 0x08)
		outb(channels[channel].base  + reg - 0x00, data);
	else if (reg < 0x0C)
		outb(channels[channel].base  + reg - 0x06, data);
	else if (reg < 0x0E)
		outb(channels[channel].ctrlBase  + reg - 0x0A, data);
	else if (reg < 0x16)
		outb(channels[channel].busMasterIDE + reg - 0x0E, data);
	if (reg > 0x07 && reg < 0x0C)
		ATA_write(channel, ATA_REG_CONTROL, channels[channel].noInt); // not sure what it does
}

uint8_t ATA_read(uint8_t channel, uint8_t reg){
	// to understand this reg translation see REG info
	uint8_t result;

	if (reg > 0x07 && reg < 0x0C)
		ATA_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].noInt); // not sure what it does
	if (reg < 0x08)
		result = inb(channels[channel].base + reg - 0x00);
	else if (reg < 0x0C)
		result = inb(channels[channel].base  + reg - 0x06);
	else if (reg < 0x0E)
		result = inb(channels[channel].ctrlBase  + reg - 0x0A);
	else if (reg < 0x16)
		result = inb(channels[channel].busMasterIDE + reg - 0x0E);
	if (reg > 0x07 && reg < 0x0C)
		ATA_write(channel, ATA_REG_CONTROL, channels[channel].noInt); // not sure what it does

	return result;
}

void ATA_init(){
	channels[ATA_PRIMARY].base = 0x1F0;
	channels[ATA_PRIMARY].ctrlBase = 0x3F6;
	channels[ATA_PRIMARY].busMasterIDE = 0; // ??
	channels[ATA_PRIMARY].noInt = 2; // ??

	channels[ATA_SECONDARY].base = 0x170;
	channels[ATA_SECONDARY].ctrlBase = 0x376;
	channels[ATA_SECONDARY].busMasterIDE = 8; // ??
	channels[ATA_SECONDARY].noInt = 2; // ??

	// Disable interrupts
	ATA_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2); // disable interrupts
   	ATA_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

   	// Detect ATA-ATAPI Devices:
   	int count = -1;
   	for(int i = 0; i < 2; i++)
    	for(int j = 0; j < 2; j++){
    		count++;

    		unsigned char err = 0;
    		uint16_t type = IDE_UNKNOWN;
    		ideDevices[count].type = IDE_UNKNOWN;
    		uint8_t status;
    		ideDevices[count].exists = false; // Assuming that no drive here.
 	
    		// Select Drive:
    		ATA_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
    		io_wait();
 	
    		// Send ATA Identify Command:
    		ATA_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    		io_wait();
 	
    		// Polling:
    		if (ATA_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.
 	
    		while(1){
    			status = ATA_read(i, ATA_REG_STATUS);
    			if((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
    			if(!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)){
    				type = IDE_ATA;
    				break; // Everything is right.
    			}
    		}
 
    		 // Probe for ATAPI Devices:
 
    		if(err != 0){
    			unsigned char cl = ATA_read(i, ATA_REG_LBA1);
    			unsigned char ch = ATA_read(i, ATA_REG_LBA2);
 
    			if (cl == 0x14 && ch ==0xEB)
    				type = IDE_ATAPI;
    			else if (cl == 0x69 && ch == 0x96)
    				type = IDE_ATAPI;
    			else
    				continue;
 
    			ATA_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
    			io_wait();
    		}

    		// Read Identification Space of the Device:
    		//terminal_print(debugTerminal, "Buff:\n");
	        int ideBuff[128];
	        for(int k=0; k<128; k++){
	        	ideBuff[k] = ind(channels[i].base + ATA_REG_DATA);
	        	//terminal_print(debugTerminal, "%x; ", ideBuff[k]);
	        }
	        //terminal_print(debugTerminal, "\n");
	 
	        // Read Device Parameters:
	        ideDevices[count].exists     	= true;
	        ideDevices[count].type         = type;
	        ideDevices[count].channel      = i;
	        ideDevices[count].drive        = j;
	        ideDevices[count].signature    = *((uint16_t *)(ideBuff + ATA_IDENT_DEVICETYPE));
	        ideDevices[count].capabilities = *((uint16_t *)(ideBuff + ATA_IDENT_CAPABILITIES));
	        ideDevices[count].commandSets  = *((unsigned int *)(ideBuff + ATA_IDENT_COMMANDSETS));
	 
	        // Get Size:
	        if (ideDevices[count].commandSets & (1 << 26))
	           // Device uses 48-Bit Addressing:
	           ideDevices[count].size   = *((unsigned int *)(ideBuff + ATA_IDENT_MAX_LBA_EXT));
	        else
	           // Device uses CHS or 28-bit Addressing:
	           ideDevices[count].size   = *((unsigned int *)(ideBuff + ATA_IDENT_MAX_LBA));
	 
	        // String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
	        for(int k = 0; k < 40; k += 2) {
	           ideDevices[count].model[k] = ideBuff[ATA_IDENT_MODEL + k + 1];
	           ideDevices[count].model[k + 1] = ideBuff[ATA_IDENT_MODEL + k];}
	        ideDevices[count].model[40] = 0; // Terminate String.

    	}

    	// Print summary
    	for(int i = 0; i < 4; i++){
    		if(i == 0)
	    		terminal_print(debugTerminal, "Primary master:\n");
	    	else if(i == 1)
	    		terminal_print(debugTerminal, "Primary slave:\n");
	    	else if(i == 2)
	    		terminal_print(debugTerminal, "Secondary master:\n");
	    	else if(i == 3)
	    		terminal_print(debugTerminal, "Secondary slave:\n");

	    	if (ideDevices[i].exists == 1){
	    		terminal_print(debugTerminal, "\tFound %s Drive %dGB - %s\n",
	    		 	(const char *[]){"ATA", "ATAPI"}[ideDevices[i].type],
	    		 	ideDevices[i].size / 1024 / 1024 / 2,
	    		 	ideDevices[i].model);
	    	}
	    	else
	    		terminal_print(debugTerminal, "\tNo drive\n");
	    }
	    terminal_print(debugTerminal, "ATA drives ready!");
	    		
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