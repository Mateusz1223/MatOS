#include "inc/HAL.h"

typedef struct ATADevice ATADevice;

struct ATADevice{
    bool  exists;    // 0 (Empty) or 1 (This Drive really exists).
    uint8_t  channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
    uint8_t  drive;       // 0 (Master Drive) or 1 (Slave Drive).
    uint8_t type;        // 0: ATA, 1:ATAPI.
    uint16_t signature;   // Drive Signature
    uint16_t capabilities;// Features.
    uint32_t   commandSets; // Command Sets Supported.
    unsigned int   size;        // Size in Sectors.
    char  model[41];   // Model in string.
}ATADevices[4];

void ATA_init();

int ATA_read_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer);
//int ATA_write_pio(ATADevice *device, uint32_t LBA, unsigned int count, uint8_t *buffer);