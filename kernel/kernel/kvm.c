#include "x86.h"
#include "device.h"
#include<elf.h>
SegDesc gdt[NR_SEGMENTS];
TSS tss;
extern void initProc();
#define SECTSIZE 512

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt));

	/*
	 * 初始化TSS
	 */
	// KSEL = kernel segment selector
	//gdt = Global Descriptor Table
	tss.ss0 = KSEL(SEG_KDATA);
	tss.esp0 = (128 << 20);
	initProc();

	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/
	// use asm in c ds es ss ljmp
	
	asm volatile("movw %w0,%%ds" :: "r"(KSEL(SEG_KDATA)));
	asm volatile("movw %w0,%%es" :: "r"(KSEL(SEG_KDATA)));
	
	asm volatile("movw %w0,%%ss" :: "r"(KSEL(SEG_KDATA)));
	//为什么堆栈段和附加段段寄存器赋值和数据段一样？ %%%%%%%%%%

	lLdt(0);
	
}

void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * */

	// USEL = user segment selector
	//entry = eip
	asm volatile("movw %w0,%%ds" :: "r"(USEL(SEG_UDATA)));
	asm volatile("movw %w0,%%es" :: "r"(USEL(SEG_UDATA)));
	
	//asm volatile("movw %0,%%ss" :: "r"(KSEL(SEG_UDATA)));
	//push ss
	asm volatile("pushw %w0" :: "r"(USEL(SEG_UDATA)));
	// push esp
	asm volatile("pushl %0" :: "i"(127 << 20));
	
	//push eflags

	asm volatile("pushfl");
	
	//push  cs
	asm volatile("pushl %0" :: "r"(USEL(SEG_UCODE)));

	//push eip
	asm volatile("pushl %0" :: "r"(entry));	
	// and use 'iret' to jump to ring3
	 
	asm volatile("iret");
}

void loadUMain(void) {

	/*加载用户程序至内存*/
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;
	char  buf[20 * 512];
	//readSect %%%%%%%%%%%%%%%%%  now just read 100 Sec
	for(int i = 0;i < 20 ; i++)
	{
		readSect(buf + i * 512,201 + i);
	}

	elf = (void *)buf;
	/* Load each program segment */
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
	for(; ph < eph; ph ++) {
		if(ph->p_type == PT_LOAD) {
			char *src;
			char *dest;
			for(int i = 0;i < ph->p_filesz ;i++)
				{
					dest = (void *)ph->p_vaddr + i;
					src = (void *)buf + ph->p_offset + i;
					*dest = *src;
				}
			for(int i = ph->p_filesz; i < ph->p_memsz;i++)
				{
					dest  = (void *)(ph->p_vaddr + i);
					*dest = (char )0;
				}

		}
	}
	volatile uint32_t entry = elf->e_entry;
	asm volatile("sti"); // 开启中断
	enterUserSpace(entry);
}

void set_tss_esp(uint32_t data)
{
	tss.esp0 = data;
}

void set_gdt(uint32_t data)
{
	gdt[SEG_UCODE].base_15_0 = data & 0xffff;
	gdt[SEG_UCODE].base_23_16 = 0xff & (data >> 16);
	gdt[SEG_UCODE].base_31_24 = data >> 24;

	gdt[SEG_UDATA].base_15_0 = data & 0xffff;
	gdt[SEG_UDATA].base_23_16 = 0xff & (data >> 16);
	gdt[SEG_UDATA].base_31_24 = data >> 24;
}