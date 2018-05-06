#include "x86.h"
#include "device.h"
#include<sys/syscall.h>
#include<sys/types.h>
#include<unistd.h>
//#define MAX_STACK_SIZE 1024 * 4
#define MAX_PCB_NUM 2 // father process , child processh




/*struct TrapFrame {
	uint32_t gs, fs , es ds;
	uint32_t edi, esi ,ebp ,esp,ebx edx , ecx, eax;
	uint32_t irq;
	uint32_t error;
	uint32_t eip , cs , eflags , esp , ss;
}*/ //已有定义，无需重复定义



struct ProcessTable PCB[MAX_PCB_NUM];
struct ProcessTable idle;
struct ProcessTable *index;
void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);
void Timer();
void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	
	asm volatile("movw %w0,%%ds" :: "r"(KSEL(SEG_KDATA)));
	asm volatile("movw %w0,%%es" :: "r"(KSEL(SEG_KDATA)));
	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x20:
			Timer(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
	index->tf = *tf; // save old trapframe


	if(PCB[0].state == RUNABLED && PCB[1].state != RUNABLED)
			index = &PCB[0];
	else if(PCB[0].state != RUNABLED && PCB[1].state == RUNABLED)
			index = &PCB[1];
	else if(PCB[0].state == RUNABLED && PCB[1].state == RUNABLED)
			{
				if(index == &PCB[0])
					index = &PCB[1];
				else if (index == &PCB[1])
					index = &PCB[0];
			}
	else index = &idle;

	//switch process
	if(index != &idle)
	{
		set_tss_esp((uint32_t)index + 1);
		set_gdt(0);
	}
	

	// set new process stack and gdt table

}

uint32_t sys_write(int fd , char * buf, uint32_t len)
{
	if(fd == STDOUT_FILENO || fd == STDERR_FILENO)
	{
		static uint32_t row = 0;
		static uint32_t col = 0;

		for(uint32_t i = 0; i < len; i++)
		{
			if(buf[i] == '\n'){
				col = 0;
				row = row + 1;	
			}
			else{
				uint32_t pos = (row * 80 + col) << 1;
				*(uint16_t *)(0xb8000 + pos) = (12 << 8)|buf[i];
				col = col + 1;
				if(col == 80)
				{
					col = 0;
					row++;
				}
			}
		}
		return len;
	}
	else 
	{
		return -1;
	}
}

uint32_t sys_fork()
{
	//在理解时，你可以认为fork后，这两个相同的虚拟地址指向的是不同的物理地址，这样方便理解父子进程之间的独立性）
//	memcpy((void *)0x300000, (void *)0x200000, 0x100000); // I don't know 
	char *dest = (void *)0x300000;
	char *src = (void *)0x200000;
	for(int i = 0; i< 0x100000;i++)
		{
			*dest = *src;
			dest++;
			src++;
		}
	// CLONE PROCESS
	PCB[1].state = PCB[0].state;
	for(int i = 0 ;i> MAX_STACK_SIZE;i++)
		PCB[1].stack[i] = PCB[0].stack[i];
	PCB[1].tf.cs = PCB[0].tf.cs;
	PCB[1].tf.ds = PCB[0].tf.ds;
	PCB[1].tf.es = PCB[0].tf.es;
	PCB[1].tf.fs = PCB[0].tf.fs;
	PCB[1].tf.gs = PCB[0].tf.gs;
	PCB[1].tf.eip = PCB[0].tf.eip;
	PCB[1].tf.eflags = PCB[0].tf.eip;
	PCB[1].tf.ebp = PCB[0].tf.ebp;
	PCB[1].tf.eax = 0;;
	PCB[1].tf.ebx = PCB[0].tf.ebx;
	PCB[1].tf.ecx = PCB[0].tf.ecx;
	PCB[1].tf.edx = PCB[0].tf.edx;
	
	//PCB[1].pid = PCB[0].pid + 1;
	//PCB[0]
	return 1;
}

uint32_t sys_exit()
{
	index ->state = DEAD;
	return 0;
}

uint32_t sys_sleep(uint32_t time)
{
	index->sleeptime = time;
	index->state = BLOCKED;
	return 0;
}

void syscallHandle(struct TrapFrame *tf) {
	
	if(tf->eax == SYS_write)
	{
		tf->eax = sys_write(tf->ebx, (char *)(tf->ecx),tf->edx);
		
	}
	else if (tf->eax == SYS_fork)
	{
		tf->eax = sys_fork();
	}
	else if(tf->eax == SYS_sleep)
		tf->eax = sys_sleep(tf->ebx);
	else if(tf->eax == SYS_exit)
		tf->eax = sys_exit();
	else assert(0);
	/* 实现系统调用*/

}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}

void Timer(struct TrapFrame *tf)
{
	if(PCB[0].state == BLOCKED)
		PCB[0].sleeptime = PCB[0].sleeptime - 1;
	if(PCB[0].sleeptime == 0)
		PCB[0].state = RUNABLED;
	// 当时间片耗尽，进程状态切换

	if(PCB[1].state == BLOCKED)
		PCB[1].sleeptime = PCB[1].sleeptime - 1;
	if(PCB[1].sleeptime == 0)
		PCB[1].state = RUNABLED;
	
}