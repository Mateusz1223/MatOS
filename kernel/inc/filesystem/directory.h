#pragma once

#include "inc/common.h"
#include "inc/UI/terminal.h"

#define MAX_DIRECTORY_ENTRIES_NUM 256

typedef struct DirectoryEntry{
	unsigned int nameLength;
	char *name;

	unsigned int size;

	uint8_t attributes;

	uint32_t firstCluster;

	uint16_t creationTime;
	/*
	Hour	5 bits
	Minutes	6 bits
	Seconds	5 bits
	*/
	uint16_t accesDate;
	/*
	Hour	5 bits
	Minutes	6 bits
	Seconds	5 bits
	*/
	uint16_t modificationTime; // same format as above
	uint16_t modificationDate; // same format as above

} DirectoryEntry;

typedef struct Directory{
	/*unsigned int pathLength;
	char *path;*/

	unsigned int entriesNum;
	DirectoryEntry *entries[MAX_DIRECTORY_ENTRIES_NUM];
} Directory;

typedef struct FileEntry{
	uint32_t firstCluster;
	uint32_t size;
	
} FileEntry;

void directory_init(Directory *this);

void directory_clear(Directory *this);

int directory_add_entry(Directory *this, char *name, unsigned int nameLength, unsigned int size, uint8_t attributes, uint32_t firstCluster,
									uint16_t creationTime, uint16_t accesDate, uint16_t modificationTime, uint16_t modificationDate);
void directory_print(Directory *this, Terminal *term);

int directory_find_directory(Directory *this, char *name); // returns directory first cluster

FileEntry directory_find_file(Directory *this, char *name); // returns file first cluster and file size
