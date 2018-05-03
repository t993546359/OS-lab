#include "x86.h"
#include "device.h"
#include<sys/syscall.h>
#include<sys/types.h>
#include<unistd.h>
void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

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
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
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
void syscallHandle(struct TrapFrame *tf) {
	
	if(tf->eax == SYS_write)
	{
		tf->eax = sys_write(tf->ebx, (char *)(tf->ecx),tf->edx);
		
	}
	else assert(0);
	/* 实现系统调用*/

}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
