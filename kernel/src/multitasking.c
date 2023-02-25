#include "inc/common.h"
#include "inc/multitasking.h"

#include "inc/UI/terminal.h"
#include "inc/drivers/VGA.h"

//#include "inc/memory/memory_manager.h"
#include "inc/memory/heap.h"

#define MAX_TASKS_NUM 256

typedef struct Task{
	uint8_t stack[4096];
	CPUState *cpuState;
} Task;

static struct TaskManagerStruct{
	Task *tasks[256];
	int tasksNum;
	int currTask;
}TaskManager;

//-------------------------------------------------------------------------

static void task_init(Task *this, void entrypoint()){ // GDT in the future
	this->cpuState = (CPUState *)(this->stack + 4096 - sizeof(CPUState));

	this->cpuState->eax = 0;
    this->cpuState->ebx = 0;
    this->cpuState->ecx = 0;
    this->cpuState->edx = 0;

    this->cpuState->esi = 0;
    this->cpuState->edi = 0;
    this->cpuState->ebp = 0;
    
    /*
    this->cpuState->gs = 0;
    this->cpuState->fs = 0;
    this->cpuState->es = 0;
    this->cpuState->ds = 0;
    */
    
    // this->cpuState->error = 0;    
   
    // this->cpuState->esp = ;
    this->cpuState->eip = (uint32_t)entrypoint;
    //this->cpuState->cs = gdt->CodeSegmentSelector();
    // this->cpuState->ss = ;
    this->cpuState->eflags = 0x202;
}

//-------------------------------------------------------------------------

void multitasking_init(){
	TaskManager.tasksNum = 0;
	TaskManager.currTask = -1;
}

CPUState *multitasking_schedule(CPUState *cpu_state){
	if(TaskManager.tasksNum <= 0)
		return cpu_state;
	if(TaskManager.currTask >= 0)
		TaskManager.tasks[TaskManager.currTask] -> cpuState = cpu_state;
	TaskManager.currTask = (TaskManager.currTask + 1) % TaskManager.tasksNum;
	return TaskManager.tasks[TaskManager.currTask] -> cpuState;
}

bool multitasking_add_task(void entrypoint()){
	if(TaskManager.tasksNum >= MAX_TASKS_NUM)
		return false;
	TaskManager.tasks[TaskManager.tasksNum] = heap_malloc(sizeof(Task));
	task_init(TaskManager.tasks[TaskManager.tasksNum], entrypoint);
	TaskManager.tasksNum++;
}

void multitasking_RTC_irq_resident(CPUState *cpu_state){

}