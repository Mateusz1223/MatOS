#pragma once
#include "inc/common.h"

#include "inc/filesystem/MBR.h"
#include "inc/filesystem/directory.h"

/* FAT
Start of the partition
	BPB
	----------
	Reserved
	----------
	FAT copies
	----------
	Data
*/

typedef struct BIOSParameterBlockFAT32 BIOSParameterBlockFAT32;

// The boot record occupies one sector, and is always placed in logical sector number zero of the "partition"
struct BIOSParameterBlockFAT32{
	// Common for all FAT types
	uint8_t jump[3];				// The first three bytes EB 3C 90 disassemble to JMP SHORT 3C NOP
	uint8_t softwareName[8];		// OEM identifier
	uint16_t bytesPerSector;		// The number of Bytes per sector (remember, all numbers are in the little-endian format).

	// Useful
	uint8_t sectorsPerCluster;		// Number of sectors per cluster.
	uint16_t reservedSectors;		// Number of reserved sectors. The boot record sectors are included in this value. (Start of the partition + reservedSectors gives us pointer to FAT table)
	uint8_t fatCopies;				// Number of File Allocation Tables (FAT's) on the storage media. Often this value is 2.
	// ------

	uint16_t rootDirEntries;		// Number of root directory entries (must be set so that the root directory occupies entire sectors)
	uint16_t totalSectors;			// The total sectors in the logical volume. If this value is 0, it means there are more than 65535 sectors in the volume, and the actual count is stored in the Large Sector Count entry at 0x20.
	uint8_t mediaType;				// This Byte indicates the media descriptor type
	uint16_t fatSectorCount;		// Number of sectors per FAT. FAT12/FAT16 only.
	uint16_t sectorsPerTrack;		// Number of sectors per track.
	uint16_t headCount;				// Number of heads or sides on the storage media.
	uint32_t hiddenSectors;			// Number of hidden sectors. (i.e. the LBA of the beginning of the partition.)
	uint32_t largeSectorCount;		// Large sector count. This field is set if there are more than 65535 sectors in the volume, resulting in a value which does not fit in the Number of Sectors entry at 0x13.

	// Extended Boot Record (FAT32 specific)

	// Useful
	uint32_t FATtableSize;				// Sectors per FAT. The size of the FAT in sectors. (fatCopies * tableSize gives us the size of the FAT table)
	// ------

	uint16_t extFlags;				// Flags
	uint16_t fatVersion;			// FAT version number. The high byte is the major version and the low byte is the minor version. FAT drivers should respect this field.

	// Useful
	uint32_t rootCluster;			// The cluster number of the root directory. Often this field is set to 2.
	// ------

	uint16_t fatInfo;				// The sector number of the FSInfo structure.
	uint16_t backupSector;			// The sector number of the backup boot sector.
	uint8_t reserved0[12];			// Reserved. When the volume is formated these bytes should be zero.
	uint8_t driveNumber;			// Drive number. The values here are identical to the values returned by the BIOS interrupt 0x13. 0x00 for a floppy disk and 0x80 for hard disks.
	uint8_t reserved; 				// Flags in Windows NT. Reserved otherwise.
	uint8_t signature;				// Signature (must be 0x28 or 0x29).
	uint32_t volumeId;				// Volume ID 'Serial' number. Used for tracking volumes between computers. You can ignore this if you want.
	uint8_t volumeLabel[11];		// Volume label string. This field is padded with spaces.
	uint8_t systemIdentifier[8];	// System identifier string. Always "FAT32   ". The spec says never to trust the contents of this string for any use.
	uint8_t bootCode[420];			// Boot code.
	uint16_t bootSignature;			// Bootable partition signature 0xAA55.

}__attribute__((packed));

typedef struct DirectoryEntryFAT32 DirectoryEntryFAT32;

struct DirectoryEntryFAT32{
	uint8_t name[11];			// 8.3 file name nad extension.
	uint8_t attributes;			// READ_ONLY=0x01 HIDDEN=0x02 SYSTEM=0x04 VOLUME_ID=0x08 DIRECTORY=0x10 ARCHIVE=0x20 LFN=READ_ONLY|HIDDEN|SYSTEM|VOLUME_ID (LFN means that this entry is a long file name entry)
	uint8_t reserved;			// Reserved for use by Windows NT.
	uint8_t cTimeTenth;			// Creation time in tenths of a second. Range 0-199 inclusive. Based on simple tests, Ubuntu16.10 stores either 0 or 100 while Windows7 stores 0-199 in this field.
	/*
	Hour	5 bits
	Minutes	6 bits
	Seconds	5 bits
	*/
	uint16_t cTime;				// The time that the file was created. Multiply Seconds by 2.
	/*
	Year	7 bits
	Month	4 bits
	Day	5 bits
	*/
	uint16_t cDate;				// The date on which the file was created.
	uint16_t aDate;				// Last accessed date. Same format as the creation date.
	uint16_t firstClusterHi;	//	The high 16 bits of this entry's first cluster number. For FAT 12 and FAT 16 this is always zero.
	uint16_t wTime;				//	Last modification time. Same format as the creation time.
	uint16_t wDate;				// Last modification date. Same format as the creation date.
	uint16_t firstClusterLow;	// The low 16 bits of this entry's first cluster number. Use this number to find the first cluster for this entry.
	uint32_t size;				// The size of the file in bytes.

}__attribute__((packed));

// Long file name entries always have a regular 8.3 entry to which they belong. The long file name entries are always placed immediately before their 8.3 entry. Here is the format of a long file name entry.

typedef struct LongFileNamesFAT32 LongFileNamesFAT32;

struct LongFileNamesFAT32{
	uint8_t order;				// The order of this entry in the sequence of long file name entries. This value helps you to know where in the file's name the characters from this entry should be placed.
	uint16_t characters0[5];	// The first 5, 2-byte characters of this entry.
	uint8_t attribute;			// Attribute. Always equals 0x0F. (the long file name attribute)
	uint8_t type;				// Long entry type. Zero for name entries.
	uint8_t checksum;			// Checksum generated of the short file name when the file was created. The short filename can change without changing the long filename in cases where the partition is mounted on a system which does not support long filenames.
	uint16_t characters1[6];	// The next 6, 2-byte characters of this entry.
	uint16_t reserved;			// Always zero.
	uint16_t characters2[2];	// The final 2, 2-byte characters of this entry.

}__attribute__((packed));

typedef struct FATVolume{
	void *driverDevice;
	int (*read)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer);
	int (*write)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer);

	uint8_t type; // FAT32/16/12 in the future

	unsigned int startLBA;
	unsigned int length;

	BIOSParameterBlockFAT32 BPB;

} FATVolume;

int FAT_volume_init(FATVolume* this, void *device,
						int (*read)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer),
						int (*write)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer), 
						PartitionTableEntry *partEntry);

int FAT_volume_read(FATVolume *this, uint32_t LBA, unsigned int count, uint8_t *buffer);

int FAT_volume_write(FATVolume *this, uint32_t LBA, unsigned int count, uint8_t *buffer);

uint8_t *FAT_read_file(FATVolume *this, char *path, int directoryCluster); // Returns 0 if error occured or invalid path

Directory *FAT_read_directory(FATVolume *this, char *path, int directoryCluster); // Returns 0 if error occured or invalid path