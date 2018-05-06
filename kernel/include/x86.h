#ifndef __X86_H__
#define __X86_H__

#include "x86/cpu.h"
#include "x86/memory.h"
#include "x86/io.h"
#include "x86/irq.h"
#define MAX_STACK_SIZE 4096
struct ProcessTable{
	uint32_t stack[MAX_STACK_SIZE];
	struct TrapFrame tf;
	int state;
	int timeCount;
	int sleeptime;
	uint32_t pid;
};
#define RUNABLED 0
#define BLOCKED 1
#define DEAD 2


#define SYS_sleep 21 
#define SYS_fork 20
#define SYS_exit 22
#define SYS_write 4
#define SEM_init 5
#define SEM_post 6
#define SEM_wait 7
#define SEM_des 8

void initSeg(void);
void loadUMain(void);
void initProc(void);
void initTimer();
void set_tss_esp(uint32_t data);

void set_gdt(uint32_t data);
#endif
