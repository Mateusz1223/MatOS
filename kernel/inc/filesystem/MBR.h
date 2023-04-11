#pragma once
#include "inc/common.h"

typedef struct PartitionTableEntry PartitionTableEntry;

struct PartitionTableEntry{
	uint8_t bootable;

	uint8_t startHead;
	uint8_t startSector; // (Bits 6-7 are the upper two bits for the ending cylinder field)
	uint8_t startCylinder;

	uint8_t partitionType;

	uint8_t endHead;
	uint8_t endSector; // (Bits 6-7 are the upper two bits for the ending cylinder field)
	uint8_t endCylinder;

	uint32_t startLBA;
	uint32_t length;

}__attribute__((packed));

typedef struct MasterBootRecord MasterBootRecord;

struct MasterBootRecord{
	uint8_t bootstrap[440];
	uint32_t signature;
	uint16_t unused;

	PartitionTableEntry partitionEntries[4];
	uint16_t magicNumber;
	
}__attribute__((packed));