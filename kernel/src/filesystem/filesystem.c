#include "inc/filesystem/filesystem.h"

#include "inc/memory/heap.h"
#include "inc/drivers/VGA.h"
#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

void filesystem_init(){
	Filesystem.FATVolumesNum = 0;
}

int filesystem_add_FATVolume(void *device,
					int (*read)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer),
					int (*write)(void *device, uint32_t LBA, unsigned int count, uint8_t *buffer), 
					PartitionTableEntry *partEntry){

	if(Filesystem.FATVolumesNum >= MAX_FATVOLUMES_NUM){
		terminal_set_color(debugTerminal, LIGHT_RED);
		terminal_print(debugTerminal, "[Filesystem error] Cannot add anymore FAT volumes to filesystem\n");
		terminal_set_color(debugTerminal, LIGHT_GREEN);
		return 9; // to many FAT volumes
	}

	Filesystem.FATVolumes[Filesystem.FATVolumesNum] = heap_malloc(sizeof(FATVolume));
	int ret = 0;
	ret = FAT_volume_init(Filesystem.FATVolumes[Filesystem.FATVolumesNum], device, read, write, partEntry);
	if(ret){
		heap_free(Filesystem.FATVolumes[Filesystem.FATVolumesNum]);
		return ret;
	}
	Filesystem.FATVolumesNum++;
	terminal_print(debugTerminal, "FAT32 volume added to filesystem!\n");
	return ret;
}

// Path example: "FA:/directory1/file.txt" -> FAT volume number 0, file.txt in directory directory1
// Relative path example: "/directory1/file.txt" -> FAT volume number 0, file.txt in directory directory1
uint8_t *filesystem_read_file(char *path, char *volume, int directoryCluster){
	terminal_print(debugTerminal, "[DEBUG] Looking for file: \"%s\"\n", path);
	if(path[2] == ':'){ // Absolute path
		terminal_print(debugTerminal, "[DEBUG] Absolute path\n");
		if(path[0] == 'F'){ // Fat volume
			int volumeNumber = path[1] - 'A';
			terminal_print(debugTerminal, "[DEBUG] Fat volume number %d\n", volumeNumber);
			if(volumeNumber < 0 || volumeNumber >= Filesystem.FATVolumesNum)
				return 0;
			return FAT_read_file(Filesystem.FATVolumes[volumeNumber], &path[3], 0);
		}
		else
			return 0;
	}
	else{ // Relative path
		terminal_print(debugTerminal, "[DEBUG] Relative path\n");
		if(volume[0] == 'F'){ // Fat volume
			int volumeNumber = volume[1] - 'A';
			terminal_print(debugTerminal, "[DEBUG] Fat volume number %d\n", volumeNumber);
			if(volumeNumber < 0 || volumeNumber >= Filesystem.FATVolumesNum)
				return 0;
			return FAT_read_file(Filesystem.FATVolumes[volumeNumber], path, directoryCluster);
		}
		else
			return 0;
	}
}

Directory *filesystem_read_directory(char *path, char *volume, int directoryCluster){
	terminal_print(debugTerminal, "[DEBUG] Looking for directory: \"%s\"\n", path);
	if(path[2] == ':'){ // Absolute path
		terminal_print(debugTerminal, "[DEBUG] Absolute path\n");
		if(path[0] == 'F'){ // Fat volume
			int volumeNumber = path[1] - 'A';
			terminal_print(debugTerminal, "[DEBUG] Fat volume number %d\n", volumeNumber);
			if(volumeNumber < 0 || volumeNumber >= Filesystem.FATVolumesNum)
				return 0;
			return FAT_read_directory(Filesystem.FATVolumes[volumeNumber], &path[3], 0);
		}
		else
			return 0;
	}
	else{ // Relative path
		terminal_print(debugTerminal, "[DEBUG] Relative path\n");
		if(path[0] != '\\' && path[0] != '/')
			return 0;
		if(volume[0] == 'F'){ // Fat volume
			int volumeNumber = volume[1] - 'A';
			terminal_print(debugTerminal, "[DEBUG] Fat volume number %d\n", volumeNumber);
			if(volumeNumber < 0 || volumeNumber >= Filesystem.FATVolumesNum)
				return 0;
			return FAT_read_directory(Filesystem.FATVolumes[volumeNumber], path, directoryCluster);
		}
		else if(volume[0] == '\0'){
			Directory *directory = heap_malloc(sizeof(Directory));
			if(directory == 0)
				return 0;
			directory_init(directory);
			for(int i=0; i<Filesystem.FATVolumesNum; i++){
				char *name = "FA (Volume)";
				name[1] += i;
				int length = 0;
				while(name[length] != '\0')
					length++;
				directory_add_entry(directory, name, length, 0, 0x10, Filesystem.FATVolumes[i]->BPB.rootCluster,
								0, 0, 0, 0);
			}
			return directory;
		}
		else
			return 0;
	}
}