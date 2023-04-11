#pragma once

#include "inc/common.h"

#include "inc/filesystem/MBR.h"
#include "inc/filesystem/FAT.h"

#define MAX_FATVOLUMES_NUM 6

struct FilesystemStruct{
	unsigned int FATVolumesNum;
	FATVolume *FATVolumes[MAX_FATVOLUMES_NUM];

} Filesystem;

void filesystem_init();

int filesystem_add_FATVolume(void *device,
					int (*read)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer),
					int (*write)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer), 
					PartitionTableEntry *partEntry);

// Path example: "FA:/directory1/file.txt" -> FAT volume number 0, file.txt in directory directory1
// Relative path example: "FA:./directory1/file.txt" -> FAT volume number 0, file.txt in directory directory1

uint8_t *filesystem_read_file(char *path, char *volume, int directoryCluster); // Returns 0 if error occured or invalid path

Directory *filesystem_read_directory(char *path, char *volume, int directoryCluster); // Returns 0 if error occured or invalid path