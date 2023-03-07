#include "inc/drivers/busses/ATA.h"

#include "inc/HAL.h"

#include "inc/drivers/VGA.h"
#include "inc/UI/terminal.h"

// https://wiki.osdev.org/PCI_IDE_Controller -> a lot of bugs in this article
// https://wiki.osdev.org/ATA_PIO_Mode

// Registers offset
// Offset from "I/O" base
#define ATA_REG_DATA       0x00 // Read/Write PIO data bytes                                                Read/written   16-bit / 16-bit
#define ATA_REG_ERROR      0x01 // Used to retrieve any error generated by the last ATA command executed.   Read           8-bit / 16-bit
#define ATA_REG_FEATURES   0x01 // Used to control command specific interface features.                     Write          8-bit / 16-bit
#define ATA_REG_SECCOUNT0  0x02 // Number of sectors to read/write (0 is a special value).                  Read/Write     8-bit / 16-bit
#define ATA_REG_LBAlo      0x03 // This is CHS / LBA28 / LBA48 specific.                                    Read/Write     8-bit / 16-bit
#define ATA_REG_LBAmid     0x04 // Partial Disk Sector address.                                             Read/Write     8-bit / 16-bit
#define ATA_REG_LBAhi      0x05 // Partial Disk Sector address.                                             Read/Write     8-bit / 16-bit
#define ATA_REG_HDDEVSEL   0x06 // Used to select a drive and/or head. Supports extra address/flag bits.    Read/Write     8-bit / 8-bit
#define ATA_REG_COMMAND    0x07 // Used to send ATA commands to the device.                                 Write          8-bit / 8-bit
#define ATA_REG_STATUS     0x07 // Used to read the current status.                                         Read           8-bit / 8-bit
// Offset from "Control" base
#define ATA_REG_ALTSTATUS  0x00 // A duplicate of the Status Register which does not affect interrupts.     Read           8-bit / 8-bit
#define ATA_REG_CONTROL    0x00 // Used to reset the bus or enable/disable interrupts.                      Write          8-bit / 8-bit
#define ATA_REG_DEVADDRESS 0x01 // Provides drive select and head select information.                       Read           8-bit / 8-bit

// Device Control Register bits
#define ATA_CTRL_nIEN 0x2  // Set this to stop the current device from sending interrupts.
#define ATA_CTRL_SRST 0x4  // Set, then clear (after 5us), this to do a "Software Reset" on all ATA drives on a bus, if one is misbehaving.
#define ATA_CTRL_HOB  0x80 // Set this to read back the High Order Byte of the last LBA48 value sent to an IO port.

// The Command/Status Port returns a bit mask referring to the status of a channel when read:
#define ATA_SR_ERR     0x1    // Indicates an error occurred. Send a new command to clear it (or nuke it with a Software Reset).
#define ATA_SR_IDX     0x2    // Index. Always set to zero.
#define ATA_SR_CORR    0x4    // Corrected data. Always set to zero.
#define ATA_SR_DRQ     0x8    // Set when the drive has PIO data to transfer, or is ready to accept PIO data.
#define ATA_SR_SRV     0x10   // Overlapped Mode Service Request.
#define ATA_SR_DF      0x20   // Drive Fault Error (does not set ERR).
#define ATA_SR_RDY     0x40   // Bit is clear when drive is spun down, or after an error. Set otherwise.
#define ATA_SR_BSY     0x80   // Indicates the drive is preparing to send/receive data (wait for it to clear). In case of 'hang' (it never clears), do a software reset.

// Error Register
#define ATA_ER_AMNF    0x1    // Address mark not found.
#define ATA_ER_TKZNF   0x2    // Track zero not found.
#define ATA_ER_ABRT    0x4    // Aborted command.
#define ATA_ER_MCR     0x8    // Media change request.
#define ATA_ER_IDNF    0x10   // ID not found.
#define ATA_ER_MC      0x20   // Media changed.
#define ATA_ER_UNC     0x40   // Uncorrectable data error.
#define ATA_ER_BBK     0x80   // Bad Block detected.

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
   uint16_t ctrl;  // Control Base
   uint8_t ctrlReg;
} channels[2];

//___________________________________________________________________________________________________

static void long_wait(){
    for(int i=0; i<=15; i++)
        io_wait();
}

static void soft_reset(uint8_t channel){
    channels[channel].ctrlReg |= ATA_CTRL_SRST;
    outb(channels[channel].ctrl + ATA_REG_CONTROL, channels[channel].ctrlReg);
    long_wait();
    channels[channel].ctrlReg &= ~(ATA_CTRL_SRST);
    outb(channels[channel].ctrl + ATA_REG_CONTROL, channels[channel].ctrlReg);
    long_wait();
}

static void print_error(uint16_t code){
    if(code == 0)
        return;
    terminal_set_color(debugTerminal, LIGHT_RED);
    terminal_print(debugTerminal, "[ATA error] code: %x", code);
    if(code & ATA_ER_AMNF)
        terminal_print(debugTerminal, " -> Address mark not found.\n");
    if(code & ATA_ER_TKZNF)
        terminal_print(debugTerminal, " -> Track zero not found.\n");
    if(code & ATA_ER_ABRT)
        terminal_print(debugTerminal, " -> Aborted command.\n");
    if(code & ATA_ER_MCR)
        terminal_print(debugTerminal, " -> Media change request.\n");
    if(code & ATA_ER_IDNF)
        terminal_print(debugTerminal, " -> ID not found.\n");
    if(code & ATA_ER_MC)
        terminal_print(debugTerminal, " -> Media changed.\n");
    if(code & ATA_ER_UNC)
        terminal_print(debugTerminal, " -> Uncorrectable data error.\n");
    if(code & ATA_ER_BBK)
        terminal_print(debugTerminal, " -> Bad Block detected.\n");
    terminal_set_color(debugTerminal, LIGHT_GREEN);
}

#define BSY_WAIT_MAX_COUNTER 100000
static int wait_while_bsy(int channel){
    int counter = 0;
    uint8_t status = 0xff;
    while((status & ATA_SR_BSY) && counter < BSY_WAIT_MAX_COUNTER){
        status = inb(channels[channel].base + ATA_REG_STATUS);
        counter++;
    }
    if(counter >= BSY_WAIT_MAX_COUNTER)
        return 1; // Error code 1: Device not responding!
    return 0;
}

static int poll_channel(int channel){
    int ret = wait_while_bsy(channel);
    if(ret)
        return ret;
    int counter = 0;
    uint8_t error = 0;
    uint8_t status = 0x00;
    while(!(status & ATA_SR_DRQ) && counter < BSY_WAIT_MAX_COUNTER){
        status = inb(channels[channel].base + ATA_REG_STATUS);
        if(status & ATA_SR_ERR){
            error = inb(channels[channel].base + ATA_REG_ERROR);
            break;
        }
        counter++;
    }
    if(counter >= BSY_WAIT_MAX_COUNTER){
        soft_reset(channel);
        return 1; // Error code 1: Device not responding!
    }
    if(error){
        print_error(error);
        return 2; // Error code 2: Unexpected error ocured
    }
    return 0;
}

static int flush(int channel){
    // Should I choose drive first ????
    outb(channels[channel].base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    long_wait();
    if(wait_while_bsy(channel)){
        if(inb(channels[channel].base + ATA_REG_STATUS) & ATA_SR_ERR)
            print_error(inb(channels[channel].base + ATA_REG_ERROR));
        terminal_set_color(debugTerminal, LIGHT_RED);
        terminal_print(debugTerminal, "[ATA Error] error occured while flushing channel %d\n", channel);
        terminal_set_color(debugTerminal, LIGHT_GREEN);
        return 1;
    }
    return 0;
}

static int flush_ext(int channel){
    // Should I choose drive first ????
    outb(channels[channel].base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH_EXT);
    long_wait();
    if(wait_while_bsy(channel)){
        if(inb(channels[channel].base + ATA_REG_STATUS) & ATA_SR_ERR)
            print_error(inb(channels[channel].base + ATA_REG_ERROR));
        terminal_set_color(debugTerminal, LIGHT_RED);
        terminal_print(debugTerminal, "[ATA Error] error occured while flushing channel %d\n", channel);
        terminal_set_color(debugTerminal, LIGHT_GREEN);
        return 1;
    }
    return 0;
}

static int ATA_read28_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer){
    int ch = device->channel;
    int dr = device->drive;

    // https://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO
    outb(channels[ch].base + ATA_REG_HDDEVSEL, (uint8_t)((uint8_t []){0xE0, 0xF0}[dr] | (uint8_t)((LBA >> 24) & 0x0F)));
    io_wait();
    outb(channels[ch].base + ATA_REG_ERROR, 0); // Supposedly it's useless
    io_wait();
    outb(channels[ch].base + ATA_REG_SECCOUNT0, (uint8_t)(count));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAlo, (uint8_t)LBA);
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAmid, (uint8_t)(LBA >> 8));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAhi, (uint8_t)(LBA >> 16));
    io_wait();
    outb(channels[ch].base + ATA_REG_COMMAND, (uint8_t)(ATA_CMD_READ_PIO));
    long_wait();
    
    for(int i=0; i<count; i++){
        int ret = poll_channel(ch);
        if(ret) // Error code 1: Device not responding!, Error code 2: Unexpected error occured!
            return ret; 
        insw(channels[ch].base + ATA_REG_DATA, (uint16_t *)(buffer + i*512), 256);
    }
    long_wait();

    return 0;
}

static int ATA_read48_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer){
    int ch = device->channel;
    int dr = device->drive;

    // https://wiki.osdev.org/ATA_PIO_Mode#48_bit_PIO
    outb(channels[ch].base + ATA_REG_HDDEVSEL, (uint8_t []){0x40, 0x50}[dr]);
    io_wait();
    outb(channels[ch].base + ATA_REG_ERROR, 0); // Supposedly it's useless
    io_wait();
    outb(channels[ch].base + ATA_REG_SECCOUNT0, (uint8_t)(count >> 8)); // high sector count byte
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAlo, (uint8_t)(LBA >> 24));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAmid, 0);
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAhi, 0);
    io_wait();
    outb(channels[ch].base + ATA_REG_SECCOUNT0, (uint8_t)(count)); // low sector count byte
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAlo, (uint8_t)LBA);
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAmid, (uint8_t)(LBA >> 8));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAhi, (uint8_t)(LBA >> 16));
    io_wait();
    outb(channels[ch].base + ATA_REG_COMMAND, (uint8_t)(ATA_CMD_READ_PIO_EXT));
    long_wait();
    
    for(int i=0; i<count; i++){
        int ret = poll_channel(ch);
        if(ret) // Error code 1: Device not responding!, Error code 2: Unexpected error occured!
            return ret; 
        insw(channels[ch].base + ATA_REG_DATA, (uint16_t *)(buffer + i*512), 256);
    }
    long_wait();

    return 0;
}

/*static int ATA_write28_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer){
    int ch = device->channel;
    int dr = device->drive;

    // https://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO
    outb(channels[ch].base + ATA_REG_HDDEVSEL, (uint8_t)((uint8_t []){0xE0, 0xF0}[dr] | (uint8_t)((LBA >> 24) & 0x0F)));
    io_wait();
    outb(channels[ch].base + ATA_REG_ERROR, 0); // Supposedly it's useless
    io_wait();
    outb(channels[ch].base + ATA_REG_SECCOUNT0, (uint8_t)(count));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAlo, (uint8_t)LBA);
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAmid, (uint8_t)(LBA >> 8));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAhi, (uint8_t)(LBA >> 16));
    io_wait();
    outb(channels[ch].base + ATA_REG_COMMAND, (uint8_t)(ATA_CMD_WRITE_PIO));
    long_wait();
    
    for(int i=0; i<count; i++){
        int ret = poll_channel(ch);
        if(ret) // Error code 1: Device not responding!, Error code 2: Unexpected error occured!
            return ret; 
        outsw(channels[ch].base + ATA_REG_DATA, (uint16_t *)(buffer + i*512), 256); // this funtion waits between out commands
    }
    flush(ch);

    return 0;
}

static int ATA_write48_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer){
    int ch = device->channel;
    int dr = device->drive;

    // https://wiki.osdev.org/ATA_PIO_Mode#48_bit_PIO
    outb(channels[ch].base + ATA_REG_HDDEVSEL, (uint8_t []){0x40, 0x50}[dr]);
    io_wait();
    outb(channels[ch].base + ATA_REG_ERROR, 0); // Supposedly it's useless
    io_wait();
    outb(channels[ch].base + ATA_REG_SECCOUNT0, (uint8_t)(count >> 8)); // high sector count byte
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAlo, (uint8_t)(LBA >> 24));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAmid, 0);
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAhi, 0);
    io_wait();
    outb(channels[ch].base + ATA_REG_SECCOUNT0, (uint8_t)(count)); // low sector count byte
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAlo, (uint8_t)LBA);
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAmid, (uint8_t)(LBA >> 8));
    io_wait();
    outb(channels[ch].base + ATA_REG_LBAhi, (uint8_t)(LBA >> 16));
    io_wait();
    outb(channels[ch].base + ATA_REG_COMMAND, (uint8_t)(ATA_CMD_WRITE_PIO_EXT));
    long_wait();
    
    for(int i=0; i<count; i++){
        int ret = poll_channel(ch);
        if(ret) // Error code 1: Device not responding!, Error code 2: Unexpected error occured!
            return ret; 
        outsw(channels[ch].base + ATA_REG_DATA, (uint16_t *)(buffer + i*512), 256); // this funtion waits between out commands
    }
    flush_ext(ch);

    return 0;
}*/

//___________________________________________________________________________________________________

void ATA_init(){
    terminal_print(debugTerminal, "Initializing ATA...\n");
    channels[0].base = 0x1F0;
    channels[0].ctrl = 0x3F6;
    channels[0].ctrlReg = ATA_CTRL_nIEN;
    channels[1].base = 0x170;
    channels[1].ctrl = 0x376;
    channels[0].ctrlReg = ATA_CTRL_nIEN;

    int id = 0;
    for(int ch=0; ch<2; ch++){
        for(int dr=0; dr<2; dr++){
            ATADevices[id].exists = false;
            ATADevices[id].channel = ch;
            ATADevices[id].drive = dr;
            id++;
        }
    }
    for(int id=0; id<4; id++){
        int ch = ATADevices[id].channel;
        int dr = ATADevices[id].drive;
        if(wait_while_bsy(ch)){
            terminal_print(debugTerminal, "Drive %d doesn't exist (1)\n", id);
            continue;
        }
        uint8_t status = inb(channels[ch].base + ATA_REG_STATUS);
        if((status & ATA_SR_IDX) || (status & ATA_SR_CORR)){ // Those bits should always be zero
            terminal_print(debugTerminal, "Drive %d doesn't exist (2)\n", id);
            continue;
        }
        // https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
        outb(channels[ch].base + ATA_REG_HDDEVSEL, (uint8_t []){0xA0, 0xB0}[dr]);
        io_wait();
        outb(channels[ch].base + ATA_REG_LBAlo, 0);
        io_wait();
        outb(channels[ch].base + ATA_REG_LBAmid, 0);
        io_wait();
        outb(channels[ch].base + ATA_REG_LBAhi, 0);
        io_wait();
        outb(channels[ch].base + ATA_REG_COMMAND, (uint8_t)(ATA_CMD_IDENTIFY));
        long_wait();
        status = inb(channels[ch].base + ATA_REG_STATUS);
        if(status == 0x00 || status == 0x6C || status == 0xEC){ // 0x00 for emulators, 0x6c for machines with resistors attached to BSY bit and 0xEC for machines without the resistor
            terminal_print(debugTerminal, "Drive %d doesn't exist (3)\n", id);
            continue;
        }
        terminal_print(debugTerminal, "Drive %d seems to exist\n", id);
        uint8_t type = IDE_ATA;
        // Polling
        int ret = poll_channel(ch);
        if(ret == 1){
            terminal_print(debugTerminal, "Drive %d is not responding, skipping the device...\n", id);
            continue;
        }
        else if(ret == 2){
            uint8_t LBAmid = inb(channels[ch].base + ATA_REG_LBAmid);
            uint8_t LBAhi = inb(channels[ch].base + ATA_REG_LBAhi);
            if (LBAmid == 0x14 && LBAhi == 0xEB)
                type = IDE_ATAPI;
            else if (LBAmid == 0x69 && LBAhi == 0x96)
                type = IDE_ATAPI;
            else{
                terminal_print(debugTerminal, "\t-> unknown type of a device. Running soft reset on the channel and skipping the device\n", id);
                soft_reset(ch);
                continue; // Unknown Type (may not be a device).
            }
            terminal_print(debugTerminal, "\t-> it's ATAPI device\n", id);
            outb(channels[ch].base + ATA_REG_COMMAND, (uint8_t)(ATA_CMD_IDENTIFY_PACKET));
            long_wait();
            ret = poll_channel(ch);
            if(ret){
                terminal_print(debugTerminal, "\t-> unexpected error, running soft reset and skipping the device\n", id);
                soft_reset(ch);
                continue;
            }
        }
        // Reading data
        char data_buffer[512];
        insw(channels[ch].base + ATA_REG_DATA, (uint16_t *)data_buffer, 256);

        ATADevices[id].exists = true;
        ATADevices[id].type = type;
        // Undefined behaviour. I should correct it (Pointer casts mangling)
        ATADevices[id].signature = *((uint16_t *)(data_buffer + ATA_IDENT_DEVICETYPE));
        ATADevices[id].capabilities = *((uint16_t *)(data_buffer + ATA_IDENT_CAPABILITIES));
        ATADevices[id].commandSets = *((uint32_t *)(data_buffer + ATA_IDENT_COMMANDSETS));

        if(ATADevices[id].commandSets & (1 << 26)) // Device uses 48-Bit Addressing
            ATADevices[id].size = *((int *)(data_buffer + ATA_IDENT_MAX_LBA_EXT));
        else // Device uses CHS or 28-bit Addressing
            ATADevices[id].size = *((int *)(data_buffer + ATA_IDENT_MAX_LBA));

        for(int k=0; k<40; k+=2){
            ATADevices[id].model[k] = data_buffer[ATA_IDENT_MODEL + k + 1];
            ATADevices[id].model[k + 1] = data_buffer[ATA_IDENT_MODEL + k];
        }
        int end = 40;
        for(int i=39; i>=0; i--)
            if(ATADevices[id].model[i] > 32){ // Not a white character
                end = i+1;
                break;
            }
        ATADevices[id].model[end] = 0; // Terminate string

        terminal_print(debugTerminal, "\t-> size: %uGB, type: %s, model: %s\n",
            ATADevices[id].size / 1024 / 1024 / 2,
            (char *[]){"ATA", "ATAPI"}[ATADevices[id].type],
            ATADevices[id].model);
    }
    terminal_print(debugTerminal, "[X] ATA ready!\n");
}

int ATA_read_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer){
    if(!device->exists)
        return 3; // Error code 3: Device does not exist
    if(wait_while_bsy(device->channel))
        return 4; // Error code 4: Device is not responding
    if(device->commandSets & (1 << 26)){ // Device uses 48-Bit Addressing
        terminal_print(debugTerminal, "[DEBUG] read LBA48\n");
        if((LBA + count > device->size) || count > 0xFFFF)
            return 5; // Error code 5: Inapropriate LBA and count
        return ATA_read48_pio(device, LBA, count, buffer);
    }
    else{
        terminal_print(debugTerminal, "[DEBUG] read LBA28\n");
        if((LBA + count > device->size) || LBA > 0xfffffff || count > 0xFF)
            return 5; // Error code 5: Inapropriate LBA and count
        return ATA_read28_pio(device, LBA, count, buffer);
    }
}

/*int ATA_write_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer){
    if(!device->exists)
        return 3; // Error code 3: Device does not exist
    if(wait_while_bsy(device->channel))
        return 4; // Error code 4: Device is not responding
    if(device->commandSets & (1 << 26)){ // Device uses 48-Bit Addressing
        terminal_print(debugTerminal, "[DEBUG] write LBA48\n");
        if((LBA + count > device->size) || count > 0xFFFF)
            return 5; // Error code 5: Inapropriate LBA and count
        return ATA_write48_pio(device, LBA, count, buffer);
    }
    else{
        terminal_print(debugTerminal, "[DEBUG] write LBA28\n");
        if((LBA + count > device->size) || LBA > 0xfffffff || count > 0xFF)
            return 5; // Error code 5: Inapropriate LBA and count
        return ATA_write28_pio(device, LBA, count, buffer);
    }
}*/
