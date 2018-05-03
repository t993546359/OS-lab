#include "boot.h"
#include <elf.h>
#define SECTSIZE 512

void bootMain(void) {
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph ,*eph;
	//333
	uint8_t buf[512 * 200];
	for(int i = 0 ; i < 200;i++)
		readSect((void *)buf + i * 512,i + 1);
	elf = (void *) buf;
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
	for( ; ph < eph ; ph++)
	{
		if(ph->p_type == PT_LOAD)
		{
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
					dest  = (void *)(ph->p_vaddr + i );
					*dest = (char )0;
				}
		}
	}
	void (*func_entry) ();
	func_entry = (void *)elf->e_entry;
	func_entry();
	
	while(1);
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
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
