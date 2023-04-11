#include "inc/filesystem/FAT.h"

#include "inc/memory/heap.h"
#include "inc/drivers/VGA.h"
#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

//___________________________________________________________________________________________________

static int read_cluster(FATVolume *this, int cluster, uint8_t *buffer){
	uint32_t fatStart = this->startLBA + this->BPB.reservedSectors;
	uint32_t fatSize = this->BPB.FATtableSize;
	uint32_t dataStart = fatStart + this->BPB.fatCopies * fatSize;

	int ret = 0;
	ret = FAT_volume_read(this, (dataStart + this->BPB.sectorsPerCluster * (cluster-2)), this->BPB.sectorsPerCluster, buffer);
	if(ret)
		return ret;

	return 0;
}

static int get_new_cluster(FATVolume *this, int *cluster, uint8_t *fatBuffer, int currentFatSector){
	uint32_t fatStart = this->startLBA + this->BPB.reservedSectors;
	uint32_t fatSize = this->BPB.FATtableSize;

	int fatSector = *cluster / (512/sizeof(uint32_t));
		if(fatSector != currentFatSector){
			int ret = 0;
			ret = FAT_volume_read(this, (fatStart + fatSector), 1, fatBuffer);
			if(ret)
				return ret;
			currentFatSector = fatSector;
		}
		*cluster = ((uint32_t *)&fatBuffer)[*cluster % (512/sizeof(uint32_t))]; // ????
		*cluster &= 0x0FFFFFFF;
}

static int load_directory(FATVolume *this, int firstCluster, Directory *directory){
	uint32_t fatStart = this->startLBA + this->BPB.reservedSectors;
	uint32_t fatSize = this->BPB.FATtableSize;
	uint32_t dataStart = fatStart + this->BPB.fatCopies * fatSize;

	char name[257];
	int index = 256;
	bool end = false;
	DirectoryEntryFAT32 *rootBuffer = heap_malloc(this->BPB.sectorsPerCluster * 512 + 1);
	if(rootBuffer == 0)
			return 7; // Could not allocate memory for the root buffer
	uint8_t fatBuffer[513];
	int currentFatSector = -1;
	uint32_t cluster = firstCluster;

	while(!end){
		int ret = read_cluster(this, cluster, (uint8_t *)rootBuffer);
		if(ret){
			terminal_set_color(debugTerminal, LIGHT_RED);
			terminal_print(debugTerminal, "[FAT Error] Could not read root directory cluster.\n");
			terminal_set_color(debugTerminal, LIGHT_GREEN);
			heap_free(rootBuffer);
			return ret;
		}

		for(int i=0; i<16*this->BPB.sectorsPerCluster; i++){
			if(rootBuffer[i].name[0] == 0){
				end = true;
				break;
			}
			if(rootBuffer[i].name[0] == 0xE5)
				continue;
			if(rootBuffer[i].attributes == 0x0F){ // Long file name entry
				LongFileNamesFAT32 *entry = (LongFileNamesFAT32 *)&rootBuffer[i];
				for(int j=1; j>=0; j--){
					if((char)(entry->characters2[j]) > 126 || (char)(entry->characters2[j]) < 32) // Not ASCII (ignore)
						continue;
					name[index] = (char)(entry->characters2[j]);
					index--;
				}
				for(int j=5; j>=0; j--){
					if((char)(entry->characters1[j]) > 126 || (char)(entry->characters1[j]) < 32) // Not ASCII (ignore)
						continue;
					name[index] = (char)(entry->characters1[j]);
					index--;
				}
				for(int j=4; j>=0; j--){
					if((char)(entry->characters0[j]) > 126 || (char)(entry->characters0[j]) < 32) // Not ASCII (ignore)
						continue;
					name[index] = (char)(entry->characters0[j]);
					index--;
				}
				continue;
			}
			int nameLength;
			if(index == 256){ // Only 8.3 name exists
				for(int j=0; j<11; j++)
					name[j] = rootBuffer[i].name[j];
				name[11] = '\0';
				for(int j=10; j>0; j--){
					if(name[j] == ' ')
						name[j] = '\0';
					else
						break;
				}
				nameLength = 11;
			}
			else{
				nameLength = 0;
				index++;
				for(index; index<=256; index++){
					name[nameLength] = name[index];
					nameLength++;
				}
				index = 256;
				name[nameLength] = '\0';
			}

			uint32_t firstCluster = (uint32_t)rootBuffer[i].firstClusterLow;
			firstCluster |= (uint32_t)rootBuffer[i].firstClusterHi << 16;
			directory_add_entry(directory, name, nameLength, rootBuffer[i].size, rootBuffer[i].attributes, firstCluster,
								rootBuffer[i].cTime, rootBuffer[i].aDate, rootBuffer[i].wTime, rootBuffer[i].wDate);
		}
		if(end)
			break;

		// Extract new cluster
		ret = get_new_cluster(this, &cluster, fatBuffer, currentFatSector);
		if(ret){
			heap_free(rootBuffer);
			return ret;
		}
		if(cluster >= 0x0FFFFFF8) // last cluster of the file
			break;
		if(cluster == 0x0FFFFFF7){ // bad sector
			terminal_set_color(debugTerminal, LIGHT_RED);
			terminal_print(debugTerminal, "[FAT Error] bad sector encountered while reading a directory.\n");
			terminal_set_color(debugTerminal, LIGHT_GREEN);
			break;
		}
		if(cluster < 2){
			terminal_set_color(debugTerminal, LIGHT_RED);
			terminal_print(debugTerminal, "[FAT Error] incorrect value encountered in FAT while reading a directory.\n");
			terminal_set_color(debugTerminal, LIGHT_GREEN);
			break;
		}
	}

	heap_free(rootBuffer);
	return 0;
} 

static int load_file(FATVolume *this, int firstCluster, int size, uint8_t *buffer){
	uint32_t fatStart = this->startLBA + this->BPB.reservedSectors;
	uint32_t fatSize = this->BPB.FATtableSize;
	uint32_t dataStart = fatStart + this->BPB.fatCopies * fatSize;

	uint8_t *tmpBuffer = heap_malloc(512 * this->BPB.sectorsPerCluster + 1);
	if(tmpBuffer == 0)
			return 7; // Could not allocate memory for the root buffer
	uint8_t fatBuffer[513];
	int currentFatSector = -1;

	int cluster = firstCluster;
	for(size; size>0; size-=(512*this->BPB.sectorsPerCluster)){
		// Read cluster
		int ret = 0;
		ret = FAT_volume_read(this, (dataStart + this->BPB.sectorsPerCluster * (cluster-2)), this->BPB.sectorsPerCluster, tmpBuffer);
		if(ret){
			heap_free(tmpBuffer);
			return ret;
		}

		// Copy values to the buffer (Slow. Should be changed when switching to the DMA)
		if(size <= 512*this->BPB.sectorsPerCluster){
			for(int i=0; i<size; i++){
				*buffer = tmpBuffer[i];
				buffer++;
			}
			*buffer = '\0';
		}
		else{
			for(int i=0; i<512*this->BPB.sectorsPerCluster; i++){
				*buffer = tmpBuffer[i];
				buffer++;
			}
		}

		// Extract new cluster
		ret = get_new_cluster(this, &cluster, fatBuffer, currentFatSector);
		if(ret){
			heap_free(tmpBuffer);
			return ret;
		}
		if(cluster >= 0x0FFFFFF8) // last cluster of the file
			break;
		if(cluster == 0x0FFFFFF7){ // bad sector
			heap_free(tmpBuffer);
			terminal_set_color(debugTerminal, LIGHT_RED);
			terminal_print(debugTerminal, "[FAT Error] bad sector encountered while reading a file.\n");
			terminal_set_color(debugTerminal, LIGHT_GREEN);
			return 8; // Bad sector encountered
		}
		if(cluster < 2){
			heap_free(tmpBuffer);
			terminal_set_color(debugTerminal, LIGHT_RED);
			terminal_print(debugTerminal, "[FAT Error] incorrect value encountered in FAT while reading a file.\n");
			terminal_set_color(debugTerminal, LIGHT_GREEN);
			return 9; // Incorrect value encountered
		}
	}

	heap_free(tmpBuffer);
	return 0;
}

static uint32_t extract_name_from_path(char *path, char *name){ // returns length of the name
	int index = 0;
	while(path[index] != '\0' && path[index] != '\\' && path[index] != '/'){
		name[index] = path[index];
		index++;
	}
	name[index] = '\0';
	return index;
}

//___________________________________________________________________________________________________

int FAT_volume_init(FATVolume* this, void *device,
						int (*read)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer),
						int (*write)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer), 
						PartitionTableEntry *partEntry){

	this->driverDevice = device;
	this->read = read;
	this->write = write;

	this->type = partEntry->partitionType;
	this->startLBA = partEntry->startLBA;
	this->length = partEntry->length;

	// load BPB
	int ret = 0;
	ret = FAT_volume_read(this, this->startLBA, 1, (uint8_t*)&this->BPB);
	if(ret){
		terminal_print(debugTerminal, "[FAT Error] Could not read BIOS Parameter Block.\n");
		return ret;
	}

	return ret;
}

int FAT_volume_read(FATVolume *this, uint32_t LBA, unsigned int count, uint8_t *buffer){
	return (*this->read)(this->driverDevice, LBA, count, buffer);
}

int FAT_volume_write(FATVolume *this, uint32_t LBA, unsigned int count, uint8_t *buffer){
	return (*this->write)(this->driverDevice, LBA, count, buffer);
}

uint8_t *FAT_read_file(FATVolume *this, char *path, int directoryCluster){
	terminal_print(debugTerminal, "[FAT DEBUG] Looking for file: \"%s\"\n", path);
	if(directoryCluster == 0)
		directoryCluster = this->BPB.rootCluster;
	while(path[0] == '\\' || path[0] == '/')
		path++;
	char name[257];
	name[0] = '\0';
	Directory directory;
	directory_init(&directory);
	int ret = load_directory(this, directoryCluster, &directory);
	if(ret)
		return 0;
	while(true){
		uint32_t size = extract_name_from_path(path, name);
		terminal_print(debugTerminal, "[FAT DEBUG] name: \"%s\"\n", name);
		path += size;
		if(path[0] == '\0') // reached file name
			break;
		while(path[0] == '\\' || path[0] == '/')
			path++;
		int cluster = directory_find_directory(&directory, name);
		directory_clear(&directory);
		if(cluster == 0){
			terminal_print(debugTerminal, "[FAT DEBUG] Could not find the given name in the directory.\n");
			return 0;
		}
		ret = load_directory(this, cluster, &directory);
		if(ret)
			return 0;
	}
	FileEntry entry = directory_find_file(&directory, name);
	if(entry.firstCluster == 0){
		directory_clear(&directory);
		return 0;
	}
	directory_clear(&directory);
	uint8_t *buffer = heap_malloc(entry.size + this->BPB.sectorsPerCluster*512);
	if(buffer == 0)
		return 0;
	load_file(this, entry.firstCluster, entry.size, buffer);
	return buffer;
}

Directory *FAT_read_directory(FATVolume *this, char *path, int directoryCluster){ // directoryCluster == 0 means root directory
	terminal_print(debugTerminal, "[FAT DEBUG] Looking for directory: \"%s\".\n", path);
	if(directoryCluster == 0)
		directoryCluster = this->BPB.rootCluster;
	while(path[0] == '\\' || path[0] == '/')
		path++;
	Directory *directory = heap_malloc(sizeof(Directory));
	if(directory == 0)
		return 0;
	directory_init(directory);
	int ret = load_directory(this, directoryCluster, directory);
	if(ret)
		return 0;
	char name[257];
	while(true){
		if(path[0] == '\0' || ((path[0] == '\\' || path[0] == '/') && path[1] == '\0')) // reached last directory
			break;
		uint32_t size = extract_name_from_path(path, name);
		terminal_print(debugTerminal, "[FAT DEBUG] name: \"%s\".\n", name);
		path += size;
		while(path[0] == '\\' || path[0] == '/')
			path++;
		int cluster = directory_find_directory(directory, name);
		directory_clear(directory);
		if(cluster == 0){
			terminal_print(debugTerminal, "[FAT DEBUG] Could not find the given name in the directory.\n");
			return 0;
		}
		ret = load_directory(this, cluster, directory);
		if(ret)
			return 0;
	}
	return directory;
}