#include "inc/filesystem/directory.h"

#include "inc/memory/heap.h"
#include "inc/drivers/VGA.h"
#include "inc/UI/UIManager.h"
#include "inc/UI/terminal.h"

void directory_init(Directory *this){
	this->entriesNum = 0;
}

void directory_clear(Directory *this){
	for(int i=0; i<this->entriesNum; i++){
		heap_free(this->entries[i]->name);
		heap_free(this->entries[i]);
	}
	this->entriesNum = 0;
}

int directory_add_entry(Directory *this, char *name, unsigned int nameLength, unsigned int size, uint8_t attributes, uint32_t firstCluster,
									uint16_t creationTime, uint16_t accesDate, uint16_t modificationTime, uint16_t modificationDate){
	if(this->entriesNum >= MAX_DIRECTORY_ENTRIES_NUM){
		terminal_set_color(debugTerminal, LIGHT_RED);
		terminal_print(debugTerminal, "[Filesystem error] Too many entries in the directory\n");
		terminal_set_color(debugTerminal, LIGHT_GREEN);
		return 8; // too many entries in the directory
	}

	this->entries[this->entriesNum] = heap_malloc(sizeof(DirectoryEntry));
	if(this->entries[this->entriesNum] == 0)
		return 9; // Heap full
	
	this->entries[this->entriesNum]->name = heap_malloc(nameLength+1);
	if(this->entries[this->entriesNum]->name == 0){
		heap_free(this->entries[this->entriesNum]);
		return 9; // Heap full
	}
	this->entries[this->entriesNum]->nameLength = nameLength;
	for(int i=0; i<nameLength; i++)
		this->entries[this->entriesNum]->name[i] = name[i];
	this->entries[this->entriesNum]->name[nameLength] = '\0';

	this->entries[this->entriesNum]->size = size;
	this->entries[this->entriesNum]->attributes = attributes;
	this->entries[this->entriesNum]->firstCluster = firstCluster;
	this->entries[this->entriesNum]->creationTime = creationTime;
	this->entries[this->entriesNum]->accesDate = accesDate;
	this->entries[this->entriesNum]->modificationTime = modificationTime;
	this->entries[this->entriesNum]->modificationDate = modificationDate;

	this->entriesNum++;
}

void directory_print(Directory *this, Terminal *term){
	for(int i=0; i<this->entriesNum; i++){
		if(this->entries[i]->attributes & 0x10)
			terminal_print(term, "<Directory>\t%s\n", this->entries[i]->name);
		else
			terminal_print(term, "<File>     \t%s\t(Size: %dB)\n", this->entries[i]->name, this->entries[i]->size);		
	}
}

int directory_find_directory(Directory *this, char *name){
	for(int i=0; i<this->entriesNum; i++){
		if(this->entries[i]->attributes & 0x10 && strcmp(name, this->entries[i]->name)){
			return this->entries[i]->firstCluster;
		}		
	}
	return 0;
}

FileEntry directory_find_file(Directory *this, char *name){
	for(int i=0; i<this->entriesNum; i++){
		if(!(this->entries[i]->attributes & 0x10) && strcmp(name, this->entries[i]->name)){
			FileEntry ret;
			ret.firstCluster = this->entries[i]->firstCluster;
			ret.size = this->entries[i]->size;
			return ret;
		}		
	}
	FileEntry ret;
	ret.firstCluster = 0;
	ret.size = 0;
	return ret;
}