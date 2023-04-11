#include "inc/drivers/system.h"

#include "inc/memory/memory_menager.h"
#include "inc/drivers/busses/ATA.h"

#include "inc/UI/terminal.h"
#include "inc/drivers/VGA.h"

#include <cpuid.h> // This comes with gcc

//___________________________________________________________________________________________________

static void get_cpu_vendor(char* str){ // vendoe must be an array of size 13
	unsigned int unused;
    __get_cpuid(0, &unused, (unsigned int*)&str[0], (unsigned int*)&str[8], (unsigned int*)&str[4]);

    str[12] = 0;
}

static void get_cpu_brand_string(char* str){ // vendoe must be an array of size 48
    unsigned int eax, unused;
    __get_cpuid(0x80000000, &eax, &unused, &unused, &unused);
    if(eax < 0x80000004){
        str = "Brand string not present on this CPU";
        return;
    }

    __get_cpuid(0x80000002, (unsigned int*)&str[0], (unsigned int*)&str[4], (unsigned int*)&str[8], (unsigned int*)&str[12]);
    __get_cpuid(0x80000003, (unsigned int*)&str[16], (unsigned int*)&str[20], (unsigned int*)&str[24], (unsigned int*)&str[28]);
    __get_cpuid(0x80000004, (unsigned int*)&str[32], (unsigned int*)&str[36], (unsigned int*)&str[40], (unsigned int*)&str[44]);
}

static int get_cpu_model(){
    int ebx, unused;
    __cpuid(0, unused, ebx, unused, unused);
    return ebx;
}

//___________________________________________________________________________________________________

void system_init(){
    get_cpu_vendor(System.CPUVendor);
    get_cpu_brand_string(System.CPUBrandStr);
    System.CPUModel = get_cpu_model();
    terminal_print(debugTerminal, "\nCPU info\n--------\n");
    terminal_print(debugTerminal, "CPU's vendor: %s\n", System.CPUVendor);
    terminal_print(debugTerminal, "CPU's brand string: %s\n", System.CPUBrandStr);
    terminal_print(debugTerminal, "CPU's model number: %x\n", System.CPUModel);

    terminal_print(debugTerminal, "[X] System ready!\n");
}

void system_print_informations(Terminal *term){
    terminal_print(term, "System info\n-----------\n");
    terminal_print(term, "CPU\n");
    terminal_print(term, "\tCPU's vendor: %s\n", System.CPUVendor);
    terminal_print(term, "\tCPU's brand string: %s\n", System.CPUBrandStr);
    terminal_print(term, "\tCPU's model number: %x\n", System.CPUModel);
    terminal_print(term, "Memory\n");
    terminal_print(term, "\tAvailable memory: %uMB\n", memory_get_available() / 1024 / 1024);
    terminal_print(term, "IDE bus\n");
    bool none = true;
    for(int id=0; id<4; id++){
        if(!ATADevices[id].exists)
            continue;
        none = false;
        terminal_print(term, "\tmodel: %s, type: %s, size: %uGB\n",
            ATADevices[id].model,
            (char *[]){"ATA", "ATAPI"}[ATADevices[id].type],
            ATADevices[id].size / 1024 / 1024 / 2);
    }
    if(none)
        terminal_print(term, "\tNo devices connected\n");
}