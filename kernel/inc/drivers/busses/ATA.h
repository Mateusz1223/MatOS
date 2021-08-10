#include "inc/HAL.h"


void ATA_init();

void ATA_write(uint8_t channel, uint8_t reg, uint8_t data);
uint8_t ide_read(uint8_t channel, uint8_t reg);
void ATA_check(); // to be deleted